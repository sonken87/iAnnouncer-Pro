// Interaction test for the Feedback page: `node live-feedback.js path/to/gallery [screenshot-dir]`
const { launch } = require("./harness");

(async () => {
  const [binary, shots] = process.argv.slice(2);
  const app = await launch(binary);
  const { page } = app;
  const results = [];
  const check = async (name, fn) => {
    try {
      const r = await fn();
      results.push(`${r === false ? "FAIL" : "ok  "} ${name}${typeof r === "string" ? " — " + r : ""}`);
    } catch (e) {
      results.push(`FAIL ${name} — ${e.message.split("\n")[0]}`);
    }
  };
  const caption = (re) => page.textContent(`.skeuo-caption >> text=${re}`);

  await page.click('.gallery__nav-item:has-text("Feedback")');
  await app.settle();

  await check("page opens", async () => (await page.textContent(".gallery__main h1")) === "Feedback");
  await check("inline dialog field shows its value after navigating here", async () => {
    const v = await page.inputValue(".fb-dialogs input");
    return v === "Evening news" ? true : `value=${v}`;
  });

  await check("dismissible alert: close button hides it, button brings it back", async () => {
    await page.click(".skeuo-alert__close");
    await app.settle();
    const gone = !(await page.isVisible("text=Dismiss me with the close button."));
    await page.click('.skeuo-btn:has-text("Bring the alert back")');
    await app.settle();
    const back = await page.isVisible("text=Dismiss me with the close button.");
    return gone && back ? true : `gone=${gone} back=${back}`;
  });

  await check("progress: Advance 10% updates bar, ring and caption", async () => {
    await page.click('.skeuo-btn:has-text("Advance 10%")');
    await page.click('.skeuo-btn:has-text("Advance 10%")');
    await app.settle();
    const text = await caption("/^60%$/");
    const width = await page.getAttribute(".fb-linear:has-text('interactive') .skeuo-linear__bar", "style");
    const ring = await page.getAttribute(".demo__row:has-text('Advance 10%') .skeuo-circular", "aria-valuenow");
    return text === "60%" && width === "width:60%" && ring === "60" ? true : `${text} ${width} ${ring}`;
  });

  const openDialog = async () => {
    await page.click('.skeuo-btn:has-text("Open dialog")');
    await app.settle();
  };
  const dialogOpen = () => page.isVisible(".skeuo-dialog:not(.skeuo-dialog--inline) .skeuo-dialog__paper");

  await check("dialog opens with focus inside the paper", async () => {
    await openDialog();
    const open = await dialogOpen();
    const focused = await page.evaluate(() => document.activeElement.classList.contains("skeuo-dialog__paper"));
    return open && focused ? true : `open=${open} focused=${focused}`;
  });
  if (shots) await page.screenshot({ path: `${shots}/live-feedback-dialog.png` });
  await check("dialog: Escape closes with reason escapeKeyDown", async () => {
    await page.keyboard.press("Escape");
    await app.settle();
    const closed = !(await dialogOpen());
    const text = await caption("/Last answer/");
    return closed && text.includes("dismissed (escapeKeyDown)") ? true : `closed=${closed} ${text}`;
  });
  await check("dialog: scrim click closes with reason backdropClick", async () => {
    await openDialog();
    await page.mouse.click(30, 30);
    await app.settle();
    const closed = !(await dialogOpen());
    const text = await caption("/Last answer/");
    return closed && text.includes("dismissed (backdropClick)") ? true : `closed=${closed} ${text}`;
  });
  await check("dialog: click inside the paper does not close it", async () => {
    await openDialog();
    await page.click(".skeuo-dialog:not(.skeuo-dialog--inline) .skeuo-dialog__text");
    await app.settle();
    return dialogOpen();
  });
  await check("dialog: action button answers and closes", async () => {
    await page.click('.skeuo-dialog:not(.skeuo-dialog--inline) .skeuo-btn:has-text("Discard")');
    await app.settle();
    const closed = !(await dialogOpen());
    const text = await caption("/Last answer/");
    return closed && text === "Last answer: discarded" ? true : `closed=${closed} ${text}`;
  });
  await check("dialog: Keep editing", async () => {
    await openDialog();
    await page.click('.skeuo-btn:has-text("Keep editing")');
    await app.settle();
    return !(await dialogOpen()) && (await caption("/Last answer/")) === "Last answer: kept";
  });

  await check("snackbar: Bottom left appears, Dismiss closes with 'action'", async () => {
    await page.click('.skeuo-btn:has-text("Bottom left")');
    await app.settle();
    const sel = ".skeuo-snackbar:not(.skeuo-snackbar--inline)";
    const shown = await page.isVisible(`${sel}.skeuo-snackbar--bottom.skeuo-snackbar--left`);
    await page.click(`${sel} .skeuo-btn:has-text("Dismiss")`);
    await app.settle();
    const gone = (await page.$(sel)) === null;
    const text = await caption("/Last closed by/");
    return shown && gone && text === "Last closed by: action" ? true : `shown=${shown} gone=${gone} ${text}`;
  });
  await check("snackbar: Top center auto-hides after 4 s (timeout)", async () => {
    await page.click('.skeuo-btn:has-text("Top center")');
    await page.mouse.move(640, 880);  // hovering the snackbar would pause the countdown
    await app.settle();
    const sel = ".skeuo-snackbar:not(.skeuo-snackbar--inline)";
    const shown = await page.isVisible(`${sel}.skeuo-snackbar--top.skeuo-snackbar--center`);
    if (shots) await page.screenshot({ path: `${shots}/live-feedback-snackbar.png` });
    await page.waitForTimeout(2500);
    const still = (await page.$(sel)) !== null;
    await page.waitForTimeout(2200);
    const gone = (await page.$(sel)) === null;
    const text = await caption("/Last closed by/");
    return shown && still && gone && text === "Last closed by: timeout" ? true
      : `shown=${shown} still=${still} gone=${gone} ${text}`;
  });
  await check("snackbar: re-showing restarts the countdown", async () => {
    await page.click('.skeuo-btn:has-text("Bottom right")');
    await page.mouse.move(640, 10);
    await page.waitForTimeout(3000);
    await page.click('.skeuo-btn:has-text("Bottom left")');
    await page.mouse.move(640, 10);
    await page.waitForTimeout(2000);  // 5 s after the first show, 2 s after the second
    const sel = ".skeuo-snackbar:not(.skeuo-snackbar--inline)";
    const still = await page.isVisible(`${sel}.skeuo-snackbar--left`);
    await page.waitForTimeout(2500);
    const gone = (await page.$(sel)) === null;
    return still && gone ? true : `still=${still} gone=${gone}`;
  });

  const backdrop = ".skeuo-backdrop:not(.skeuo-backdrop--inline)";
  await check("backdrop: shows and a click dismisses it", async () => {
    await page.click('.skeuo-btn:has-text("Show backdrop")');
    await app.settle();
    const shown = await page.isVisible(backdrop);
    await page.mouse.click(300, 300);
    await app.settle();
    const gone = (await page.$(backdrop)) === null;
    return shown && gone ? true : `shown=${shown} gone=${gone}`;
  });

  const modal = ".skeuo-modal";
  const openModal = async () => {
    await page.click('.skeuo-btn:has-text("Open modal")');
    await app.settle();
  };
  await check("modal: opens focused, Close button closes", async () => {
    await openModal();
    const shown = await page.isVisible(`${modal} .fb-modal-panel`);
    const focused = await page.evaluate(() => document.activeElement.classList.contains("skeuo-modal__content"));
    await page.click(`${modal} .skeuo-btn:has-text("Close")`);
    await app.settle();
    const gone = (await page.$(modal)) === null;
    return shown && focused && gone ? true : `shown=${shown} focused=${focused} gone=${gone}`;
  });
  await check("modal: Escape closes", async () => {
    await openModal();
    await page.keyboard.press("Escape");
    await app.settle();
    return (await page.$(modal)) === null;
  });
  await check("modal: scrim click closes", async () => {
    await openModal();
    await page.mouse.click(20, 20);
    await app.settle();
    return (await page.$(modal)) === null;
  });

  console.log(results.join("\n"));
  console.log(`${app.exchanges} messages exchanged; ${app.errors.length ? "page errors: " + app.errors.join(" | ") : "no page errors"}`);
  await app.close();
  process.exit(results.some((r) => r.startsWith("FAIL")) ? 1 : 0);
})();
