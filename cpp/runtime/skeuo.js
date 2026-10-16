// skeuo-ui for C++ — the page runtime.
//
// Delegates DOM events to the C++ handlers referenced by `data-sk-on`
// ("click:12,keydown:13!~"; ! = stop propagation, ~ = prevent default), sends
// them to the host one at a time, and morphs the returned HTML into the page
// so focus, caret position, scroll and running CSS animations survive.
(() => {
  "use strict";
  const root = document.getElementById("sk-root");

  // ---------------------------------------------------------------- transport
  // send(json) -> Promise<json reply>
  let send = null;
  if (window.chrome && window.chrome.webview) {
    // WebView2: replies come back in order as web messages.
    const waiting = [];
    window.chrome.webview.addEventListener("message", (e) => {
      const resolve = waiting.shift();
      if (resolve) resolve(typeof e.data === "string" ? e.data : JSON.stringify(e.data));
    });
    send = (msg) => new Promise((resolve) => {
      waiting.push(resolve);
      window.chrome.webview.postMessage(msg);
    });
  } else if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.skeuo) {
    // WebKit hosts: the host calls window.__skReply(json) for each message.
    const waiting = [];
    window.__skReply = (json) => {
      const resolve = waiting.shift();
      if (resolve) resolve(json);
    };
    send = (msg) => new Promise((resolve) => {
      waiting.push(resolve);
      window.webkit.messageHandlers.skeuo.postMessage(msg);
    });
  } else if (typeof window.__skHostSend === "function") {
    // Test harness (tools/harness.js) bridging to the stdio host.
    send = (msg) => Promise.resolve(window.__skHostSend(msg));
  }

  // ---------------------------------------------------------------- events
  const DOM_TYPES = ["click", "dblclick", "input", "change", "keydown", "keyup", "focusin",
                     "focusout", "mouseover", "mouseout", "animationend", "submit"];

  function listeners(el) {
    const spec = el.getAttribute && el.getAttribute("data-sk-on");
    if (!spec) return [];
    return spec.split(",").map((part) => {
      const [type, rest] = part.split(":");
      return { type, id: parseInt(rest, 10), stop: rest.includes("!"), prevent: rest.includes("~") };
    });
  }

  function matches(listenerType, e, el) {
    const t = e.type;
    if (listenerType === t) return true;
    if (listenerType === "focus") return t === "focusin";
    if (listenerType === "blur") return t === "focusout";
    if (listenerType === "mouseenter" || listenerType === "mouseleave") {
      const want = listenerType === "mouseenter" ? "mouseover" : "mouseout";
      return t === want && !(e.relatedTarget && el.contains(e.relatedTarget));
    }
    return false;
  }

  // Keydown handlers can restrict themselves to some keys with
  // data-sk-keys="ArrowUp ArrowDown Enter Escape Space"; those keys also get
  // their default action prevented (no scrolling while moving in a menu).
  function keyAllowed(el, e) {
    const keys = el.getAttribute("data-sk-keys");
    if (!keys) return { ok: true, prevent: false };
    const key = e.key === " " ? "Space" : e.key;
    const ok = keys.split(/\s+/).includes(key);
    return { ok, prevent: ok };
  }

  // The chain of (element, listener type) a DOM event reaches, bottom-up.
  function chainFor(e) {
    const chain = [];
    for (let el = e.target; el && el !== document; el = el.parentNode) {
      if (el.nodeType !== 1) continue;
      for (const l of listeners(el)) {
        if (!matches(l.type, e, el)) continue;
        if (e.type === "keydown" || e.type === "keyup") {
          const k = keyAllowed(el, e);
          if (!k.ok) continue;
          if (k.prevent) e.preventDefault();
        }
        if (l.prevent || e.type === "submit") e.preventDefault();
        chain.push({ el, type: l.type });
        if (l.stop) return chain;
      }
      if (el === root) break;
    }
    return chain;
  }

  function payload(e, first) {
    const t = e.target || {};
    const related = e.relatedTarget;
    return {
      type: first.type,
      relatedInside: !!(related && first.el.contains(related)),
      value: typeof t.value === "string" ? t.value : "",
      checked: !!t.checked,
      key: e.key || "",
      shift: !!e.shiftKey, ctrl: !!e.ctrlKey, alt: !!e.altKey, meta: !!e.metaKey,
    };
  }

  const queue = [];
  let busy = false;
  let seq = 0;
  let inputCounter = 0;
  const lastInput = new WeakMap();  // element -> inputCounter of its latest input event

  function onDomEvent(e) {
    if (!send) return;
    const chain = chainFor(e);
    if (!chain.length) return;
    if (e.type === "input") lastInput.set(e.target, ++inputCounter);
    queue.push({ chain, payload: payload(e, chain[0]), mark: inputCounter });
    pump();
  }

  // Handler ids change on every render, so they are looked up when the event
  // is sent, from the elements the event reached.
  function idsFor(chain) {
    const ids = [];
    for (const { el, type } of chain) {
      if (!el.isConnected) continue;
      const l = listeners(el).find((x) => x.type === type);
      if (l) ids.push(l.id);
    }
    return ids;
  }

  async function pump() {
    if (busy) return;
    busy = true;
    try {
      while (queue.length) {
        const item = queue.shift();
        const ids = idsFor(item.chain);
        if (!ids.length) continue;
        const reply = JSON.parse(await send(JSON.stringify({ t: "event", seq: ++seq, h: ids, e: item.payload })));
        if (reply && reply.t === "render") apply(reply.html, item.mark);
      }
    } finally {
      busy = false;
    }
  }

  // ---------------------------------------------------------------- patching
  function attrsOf(el) {
    const out = {};
    for (const a of el.attributes) out[a.name] = a.value;
    return out;
  }

  function syncProps(el, prev, next, mark) {
    const tag = el.tagName;
    if (tag !== "INPUT" && tag !== "TEXTAREA" && tag !== "SELECT") return;
    // Like Dioxus: a property is written only when the rendered attribute
    // changed, so uncontrolled fields keep what the user typed.
    if (prev.value !== next.value && next.value !== undefined) {
      const typing = (lastInput.get(el) || 0) > mark;
      if (!typing && el.value !== next.value) el.value = next.value;
    }
    if (("checked" in prev) !== ("checked" in next)) el.checked = "checked" in next;
  }

  function morphElement(o, n, mark) {
    const prev = o.__skAttrs || attrsOf(o);
    const next = attrsOf(n);
    for (const [k, v] of Object.entries(next)) if (o.getAttribute(k) !== v) o.setAttribute(k, v);
    for (const a of Array.from(o.attributes)) if (!(a.name in next)) o.removeAttribute(a.name);
    o.__skAttrs = next;
    syncProps(o, prev, next, mark);
    // Elements marked data-sk-ignore own their children (e.g. a Leaflet map):
    // only their attributes are patched.
    if (!("data-sk-ignore" in next)) morphChildren(o, n, mark);
  }

  function morphChildren(oldParent, newParent, mark) {
    const oldKids = Array.from(oldParent.childNodes);
    const newKids = Array.from(newParent.childNodes);
    for (let i = 0; i < newKids.length; i++) {
      const n = newKids[i];
      const o = oldKids[i];
      if (!o) {
        oldParent.appendChild(n);
      } else if (o.nodeType !== n.nodeType || (o.nodeType === 1 && o.tagName !== n.tagName)) {
        oldParent.replaceChild(n, o);
      } else if (o.nodeType === 1) {
        morphElement(o, n, mark);
      } else if (o.nodeValue !== n.nodeValue) {
        o.nodeValue = n.nodeValue;
      }
    }
    for (let i = newKids.length; i < oldKids.length; i++) oldParent.removeChild(oldKids[i]);
  }

  // Elements rendered with data-sk-autofocus take focus once, when they
  // appear (dialogs, menus, drawers opening).
  const focused = new WeakSet();
  function autofocus() {
    for (const el of root.querySelectorAll("[data-sk-autofocus]")) {
      if (focused.has(el)) continue;
      focused.add(el);
      el.focus({ preventScroll: true });
    }
  }

  function apply(html, mark) {
    const tpl = document.createElement("template");
    tpl.innerHTML = html;
    morphChildren(root, tpl.content, mark);
    autofocus();
  }

  for (const t of DOM_TYPES) document.addEventListener(t, onDomEvent, false);
  autofocus();
  // idle(): no event waiting for, or travelling to, C++ (used by tests).
  window.__sk = { apply, ready: !!send, idle: () => !busy && queue.length === 0 };
})();
