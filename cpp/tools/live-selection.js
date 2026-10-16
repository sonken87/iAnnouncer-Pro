// Interaction test for the Selection & forms gallery page:
//   node live-selection.js path/to/gallery [screenshot-dir]
const { launch } = require("./harness");

(async () => {
  const [binary, shots] = process.argv.slice(2);
  const app = await launch(binary);
  const { page } = app;
  const results = [];
  const check = async (name, fn) => {
    try {
      const r = await fn();
      results.push(`${r === true ? "ok  " : "FAIL"} ${name}${typeof r === "string" ? " — " + r : ""}`);
    } catch (e) {
      results.push(`FAIL ${name} — ${e.message.split("\n")[0]}`);
    }
  };
  // Polls `fn` until it returns true (events round-trip one at a time, so a
  // burst of key presses can take a moment to be patched in).
  const until = async (fn, timeout = 3000) => {
    const end = Date.now() + timeout;
    let r;
    while (Date.now() < end) {
      r = await fn();
      if (r === true) return true;
      await page.waitForTimeout(50);
    }
    return r;
  };
  const summary = () => page.textContent(".selection-demo__summary");
  const expectSummary = async (part) => {
    const s = await summary();
    return s.includes(part) ? true : `summary: ${s}`;
  };
  const plate = (title) => page.locator(`section.demo:has(h2:text-is("${title}"))`);
  const broadcast = plate("Broadcast settings");

  await page.click('.gallery__nav-item:has-text("Selection")');
  await app.settle();

  await check("page renders", async () => (await page.textContent(".gallery__main h1")) === "Selection & forms");
  await check("initial summary", () => expectSummary("On air · FM 101.7 · volume 72% · ducking 20–65%"));

  // ---------------------------------------------------------------- switch
  await check("switch (controlled) toggles label, helper and summary", async () => {
    await broadcast.locator('label:has-text("On air")').click();
    await app.settle();
    const label = await broadcast.locator(".skeuo-form-control-label__label").first().textContent();
    const helper = await broadcast.locator("text=Rehearsal: nothing leaves the studio.").isVisible();
    const cls = await broadcast.locator(".skeuo-switch").first().getAttribute("class");
    const s = await summary();
    return label === "Off air" && helper && !cls.includes("--checked") && s.startsWith("Off air") ? true
      : `label=${label} helper=${helper} cls=${cls} summary=${s}`;
  });
  await check("mic switch label follows state", async () => {
    await page.click('label:has-text("Mic muted")');
    await app.settle();
    return page.isVisible('label:has-text("Mic live")');
  });

  // ---------------------------------------------------------------- radio
  await check("radio group selection (controlled, row)", async () => {
    await broadcast.locator('label:has-text("DAB+")').click();
    await app.settle();
    const checked = await broadcast.locator(".skeuo-radio--checked input").getAttribute("value");
    const s = await expectSummary("DAB+");
    return checked === "dab" && s === true ? true : `checked=${checked} ${s}`;
  });
  await check("radio group selection with helper text", async () => {
    await page.click('label:has-text("Automation")');
    await app.settle();
    return page.isVisible("text=Selected: auto");
  });
  await check("uncontrolled radio group remembers selection", async () => {
    await page.click('label:has-text("Studio A")');
    await app.settle();
    const lbl = page.locator('label:has-text("Studio A") .skeuo-radio');
    return (await lbl.getAttribute("class")).includes("skeuo-radio--checked");
  });
  await check("disabled group ignores clicks", async () => {
    await page.click('label:has-text("Stereo")', { force: true });
    await app.settle();
    const cls = await page.locator('label:has-text("Stereo") .skeuo-radio').getAttribute("class");
    return !cls.includes("--checked") && cls.includes("--disabled") ? true : cls;
  });

  // ---------------------------------------------------------------- sliders
  await check("slider via keyboard arrows (value text + summary)", async () => {
    const input = broadcast.locator('input[aria-labelledby="bs-volume"]');
    await input.focus();
    for (let i = 0; i < 3; i++) await page.keyboard.press("ArrowRight");
    return until(async () => {
    const vt = await input.getAttribute("aria-valuetext");
    const bubble = await broadcast.locator(".skeuo-slider__value-label").first().textContent();
    const s = await expectSummary("volume 75%");
    return vt === "75%" && bubble === "75%" && s === true ? true : `valuetext=${vt} bubble=${bubble} ${s}`;
    });
  });
  await check("range slider: both thumbs by keyboard", async () => {
    await broadcast.locator('input[aria-label="Ducking window start"]').focus();
    await page.keyboard.press("ArrowRight");
    await broadcast.locator('input[aria-label="Ducking window end"]').focus();
    await page.keyboard.press("ArrowLeft");
    await page.keyboard.press("ArrowLeft");
    return until(async () => {
    const track = await broadcast.locator(".skeuo-slider--range .skeuo-slider__track").getAttribute("style");
    const s = await expectSummary("ducking 25–55%");
    // Same float arithmetic as Rust: 55 may print as 55.00000000000001.
    return /--a: 25(\.0+\d*)?%; --b: 55(\.0+\d*)?%;/.test(track) && s === true ? true : `track=${track} ${s}`;
    });
  });
  await check("continuous slider caption + vertical fader", async () => {
    await page.focus('input[aria-label="Gain"]');
    await page.keyboard.press("End");
    await app.settle(300);
    const cap = await page.locator('.demo__row:has(input[aria-label="Gain"]) .skeuo-caption').textContent();
    await page.focus('input[aria-label="Mic 1"]');
    await page.keyboard.press("ArrowDown");
    await app.settle(300);
    const fader = await page.getAttribute('input[aria-label="Mic 1"]', "aria-valuetext");
    return cap === "100" && fader === "79" ? true : `caption=${cap} fader=${fader}`;
  });
  await check("uncontrolled slider moves its thumb", async () => {
    await page.focus('input[aria-label="Inverted"]');
    await page.keyboard.press("Home");
    await app.settle(300);
    const thumb = await page.locator('.skeuo-slider:has(input[aria-label="Inverted"]) .skeuo-slider__thumb').getAttribute("style");
    return thumb.includes("--p: 0%") ? true : thumb;
  });

  // ---------------------------------------------------------------- rating
  const voice = broadcast.locator('.skeuo-rating[aria-labelledby="bs-voice"]');
  const fills = async (r) => (await r.locator(".skeuo-rating__fill").evaluateAll((els) => els.map((e) => e.style.width))).join(",");
  await check("rating click sets the value", async () => {
    await voice.locator(".skeuo-rating__hit").nth(1).click();
    await page.mouse.move(5, 5);
    await app.settle(300);
    const f = await fills(voice);
    const checked = await voice.locator("input:checked").getAttribute("value");
    return f === "100%,100%,0%,0%,0%" && checked === "2" ? true : `fills=${f} checked=${checked}`;
  });
  await check("rating click on the same value clears it", async () => {
    await voice.locator(".skeuo-rating__hit").nth(1).click();
    await page.mouse.move(5, 5);
    await app.settle(300);
    const f = await fills(voice);
    const n = await voice.locator("input:checked").count();
    return f === "0%,0%,0%,0%,0%" && n === 0 ? true : `fills=${f} checked=${n}`;
  });
  await check("rating hover previews and leaves", async () => {
    const show = page.locator('.skeuo-rating[aria-label="Show rating"]');
    const caption = page.locator('.demo__row:has(.skeuo-rating[aria-label="Show rating"]) .skeuo-caption');
    await show.locator(".skeuo-rating__hit").nth(9).hover();
    await app.settle(300);
    const during = await caption.textContent();
    const hovering = (await show.getAttribute("class")).includes("skeuo-rating--hovering");
    await page.mouse.move(5, 5);
    await app.settle(300);
    const after = await caption.textContent();
    return during === "5 · Excellent" && hovering && after === "3.5 · Good" ? true
      : `during=${during} hovering=${hovering} after=${after}`;
  });

  // ---------------------------------------------------------------- checkboxes
  const parent = broadcast.locator('label:has-text("All processing") .skeuo-checkbox');
  await check("child checkbox completes the parent (indeterminate -> checked)", async () => {
    const before = await parent.getAttribute("class");
    await broadcast.locator('label:has-text("Normalise loudness")').click();
    await app.settle();
    const after = await parent.getAttribute("class");
    const helper = await broadcast.locator("text=3 of 3 enabled").isVisible();
    return before.includes("--indeterminate") && after.includes("--checked") && !after.includes("--indeterminate") && helper
      ? true : `before=${before} after=${after} helper=${helper}`;
  });
  await check("parent checkbox clears all -> error state", async () => {
    await broadcast.locator('label:has-text("All processing")').click();
    await app.settle();
    const err = await broadcast.locator("fieldset.skeuo-form-control--error").count();
    const helper = await broadcast.locator("text=Pick at least one option.").isVisible();
    const n = await broadcast.locator(".selection-demo__nested input:checked").count();
    return err === 1 && helper && n === 0 ? true : `error=${err} helper=${helper} checked=${n}`;
  });
  await check("parent checkbox sets all", async () => {
    await broadcast.locator('label:has-text("All processing")').click();
    await app.settle();
    return (await broadcast.locator(".selection-demo__nested input:checked").count()) === 3;
  });
  await check("uncontrolled checkbox toggles", async () => {
    const cb = page.locator('.skeuo-checkbox:has(input[aria-label="Unchecked"])');
    await cb.click();
    await app.settle();
    const on = (await cb.getAttribute("class")).includes("--checked");
    await cb.click();
    await app.settle();
    const off = !(await cb.getAttribute("class")).includes("--checked");
    return on && off ? true : `on=${on} off=${off}`;
  });
  await check("controlled checkbox with caption", async () => {
    await page.click('label:has-text("I have read the station rules")');
    await app.settle();
    return page.isVisible("text=Thanks!");
  });
  await check("final summary", () => expectSummary("Off air · DAB+ · volume 75% · ducking 25–55%"));

  if (shots) await page.screenshot({ path: `${shots}/live-selection.png`, fullPage: true });
  console.log(results.join("\n"));
  console.log(`${app.exchanges} messages exchanged; ${app.errors.length ? "page errors: " + app.errors.join(" | ") : "no page errors"}`);
  await app.close();
  process.exit(results.some((r) => r.startsWith("FAIL")) || app.errors.length ? 1 : 0);
})();
