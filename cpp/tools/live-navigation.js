// Interaction test for the Navigation page: `node live-navigation.js path/to/gallery [screenshot-dir]`
const { launch } = require("./harness");

(async () => {
  const [binary, shots] = process.argv.slice(2);
  const app = await launch(binary);
  const { page } = app;
  page.setDefaultTimeout(5000);
  const results = [];
  const check = async (name, fn) => {
    try {
      const r = await fn();
      results.push(`${r === true ? "ok  " : "FAIL"} ${name}${typeof r === "string" ? " — " + r : ""}`);
    } catch (e) {
      results.push(`FAIL ${name} — ${e.message.split("\n")[0]}`);
    }
    if (process.env.VERBOSE) console.error(results[results.length - 1]);
    if (shots && results[results.length - 1].startsWith("FAIL"))
      await page.screenshot({ path: `${shots}/live-navigation-fail-${results.length}.png` }).catch(() => {});
  };
  const active = () => page.evaluate(() => {
    const a = document.activeElement;
    return a ? (a.textContent || "").trim() : "";
  });
  const key = async (k) => { await page.keyboard.press(k); await app.settle(); };
  const expect = (got, want) => (got === want ? true : `got ${JSON.stringify(got)}, want ${JSON.stringify(want)}`);

  await check("navigate to Navigation", async () => {
    await page.click('.gallery__nav-item:has-text("Navigation")');
    await app.settle();
    return expect(await page.textContent(".gallery__main h1"), "Navigation");
  });

  // ------------------------------------------------------------------ Tabs
  const studio = page.locator('[role=tablist][aria-label="Studio views"]');
  const panel = () => page.locator(".skeuo-tab-panel:not([hidden])").first().textContent();
  await check("tab click selects and switches the panel", async () => {
    await studio.locator('[role=tab]:has-text("Script")').click();
    await app.settle();
    const sel = await studio.locator('[aria-selected=true]').textContent();
    const p = await panel();
    return sel === "Script" && p.includes("6 o'clock") ? true : `selected=${sel} panel=${p}`;
  });
  await check("ArrowRight moves focus to the next tab", async () => {
    await key("ArrowRight");
    return expect(await active(), "Clips");
  });
  await check("ArrowRight skips the disabled tab and wraps", async () => {
    await key("ArrowRight");
    return expect(await active(), "Rundown");
  });
  await check("ArrowLeft wraps back (refocusing a tab focused before)", async () => {
    await key("ArrowLeft");
    const a = await active();
    await key("ArrowLeft");
    const b = await active();
    await key("ArrowRight");
    const c = await active();
    return a === "Clips" && b === "Script" && c === "Clips" ? true : `${a},${b},${c}`;
  });
  await check("End / Home jump to the ends", async () => {
    await key("End");
    const a = await active();
    await key("Home");
    const b = await active();
    return a === "Clips" && b === "Rundown" ? true : `${a},${b}`;
  });
  await check("arrow keys move focus but keep the selection", async () =>
    expect(await studio.locator('[aria-selected=true]').textContent(), "Script"));
  await check("Space on the focused tab selects it", async () => {
    await key("Space");
    const sel = await studio.locator('[aria-selected=true]').textContent();
    return sel === "Rundown" && (await panel()).includes("Seven items") ? true : `selected=${sel}`;
  });
  await check("vertical tabs use ArrowDown", async () => {
    const v = page.locator('[role=tablist][aria-label="Settings"]');
    await v.locator('[role=tab]:has-text("Output")').click();
    await app.settle();
    await key("ArrowDown");
    return expect(await active(), "Schedule");
  });
  await check("uncontrolled tabs select on click", async () => {
    const t = page.locator('[role=tablist][aria-label="Channel"]');
    await t.locator('[role=tab]:has-text("Stream")').click();
    await app.settle();
    return expect(await t.locator('[aria-selected=true]').textContent(), "Stream");
  });

  // ----------------------------------------------------------- Breadcrumbs
  await check("breadcrumbs: the … keycap expands the path", async () => {
    const bc = page.locator(".skeuo-breadcrumbs", { hasText: "06:00 News" });
    const before = await bc.locator(".skeuo-breadcrumbs__item").count();
    await bc.locator(".skeuo-breadcrumbs__collapsed").click();
    await app.settle();
    const after = await bc.locator(".skeuo-breadcrumbs__item").count();
    const week = await bc.textContent();
    return before === 3 && after === 6 && week.includes("Week 42") ? true : `before=${before} after=${after}`;
  });
  await check("button link counts clicks", async () => {
    await page.click("text=press this button link");
    await page.click("text=press this button link");
    await app.settle();
    return (await page.textContent("p:has-text('press this button link')")).includes("(2)");
  });

  // ------------------------------------------------------------------ Menu
  const trigger = page.locator("#nav-demo-menu-trigger");
  const lastAction = () => page.textContent("text=/Last action:/");
  await check("menu opens and focuses the first item", async () => {
    await trigger.click();
    await app.settle(250);
    const expanded = await trigger.getAttribute("aria-expanded");
    return expanded === "true" && (await active()).startsWith("Play") ? true : `expanded=${expanded} active=${await active()}`;
  });
  await check("menu: ArrowDown / ArrowUp / End / Home move focus", async () => {
    const seq = [];
    for (const k of ["ArrowDown", "ArrowDown", "ArrowDown", "ArrowUp", "End", "Home", "ArrowUp"]) {
      await key(k);
      seq.push((await active()).replace(/\s.*/, "").replace(/(Play|Pause|Bookmark).*/, "$1"));
    }
    return expect(seq.join(","), "Pause,Bookmark,Play,Bookmark,Bookmark,Play,Bookmark");
  });
  await check("menu: Enter chooses the focused item and closes", async () => {
    await key("ArrowUp"); // Pause
    await key("Enter");
    const last = await lastAction();
    const open = await page.locator('[role=menu][aria-labelledby="nav-demo-menu-trigger"]').count();
    return last.includes("Pause") && open === 0 ? true : `last=${last} open=${open}`;
  });
  await check("menu: Escape closes", async () => {
    await trigger.click();
    await app.settle();
    await key("Escape");
    return expect(await page.locator('[role=menu][aria-labelledby="nav-demo-menu-trigger"]').count(), 0);
  });
  await check("menu: click outside closes", async () => {
    await trigger.click();
    await app.settle();
    const opened = await page.locator('[role=menu][aria-labelledby="nav-demo-menu-trigger"]').count();
    await page.mouse.click(1200, 40);
    await app.settle();
    const after = await page.locator('[role=menu][aria-labelledby="nav-demo-menu-trigger"]').count();
    return opened === 1 && after === 0 ? true : `opened=${opened} after=${after}`;
  });
  await check("menu: hover moves focus, then the keyboard continues from there", async () => {
    await trigger.click();
    await app.settle();
    await page.locator('[role=menuitem]:has-text("Bookmark")').last().hover();
    await app.settle();
    const hovered = await active();
    await key("ArrowUp");
    const up = await active();
    await page.mouse.click(1200, 40);
    await app.settle();
    return hovered.startsWith("Bookmark") && up.startsWith("Pause") ? true : `hovered=${hovered} up=${up}`;
  });
  await check("menu: mouse click on an item chooses it", async () => {
    await trigger.click();
    await app.settle();
    const opened = await page.locator('[role=menu][aria-labelledby="nav-demo-menu-trigger"]').count();
    await page.locator('[role=menuitem]:has-text("Bookmark")').last().click();
    await app.settle();
    const last = await lastAction();
    const open = await page.locator('[role=menu][aria-labelledby="nav-demo-menu-trigger"]').count();
    return last.includes("Bookmark") && opened === 1 && open === 0 ? true : `opened=${opened} last=${last} open=${open}`;
  });
  await check("MenuList: clicking a speed moves the check", async () => {
    const list = page.locator('[aria-label="Playback speed"]');
    await list.locator('[role=menuitem]:has-text("2×")').click();
    await app.settle();
    return expect(await list.locator(".skeuo-menu-item--selected").textContent(), "2×");
  });

  // --------------------------------------------------------------- Popover
  await check("popover opens, focuses its surface, Escape closes", async () => {
    await page.click('button:has-text("Show levels")');
    await app.settle();
    const open = await page.locator('.skeuo-popover[aria-label="Levels"]').count();
    const focused = await page.evaluate(() => document.activeElement.getAttribute("aria-label"));
    await key("Escape");
    const after = await page.locator('.skeuo-popover[aria-label="Levels"]').count();
    return open === 1 && focused === "Levels" && after === 0 ? true : `open=${open} focused=${focused} after=${after}`;
  });
  await check("popover closes on outside click", async () => {
    await page.click('button:has-text("Show levels")');
    await app.settle();
    await page.mouse.click(1200, 40);
    await app.settle();
    return expect(await page.locator('.skeuo-popover[aria-label="Levels"]').count(), 0);
  });

  // ------------------------------------------------------------ Pagination
  const pager = page.locator(".demo__row", { hasText: "Page " }).locator(".skeuo-pagination");
  const caption = () => page.textContent("text=/Page \\d+ of 12/");
  await check("pagination: next / previous / number", async () => {
    await pager.locator('[aria-label="Go to next page"]').click();
    await app.settle();
    const a = await caption();
    await pager.locator('[aria-label="Go to previous page"]').click();
    await pager.locator('[aria-label="Go to previous page"]').click();
    await app.settle();
    const b = await caption();
    await pager.locator('[aria-label="Go to page 12"]').click();
    await app.settle();
    const c = await caption();
    const nextDisabled = await pager.locator('[aria-label="Go to next page"]').isDisabled();
    const current = await pager.locator('[aria-current=page]').textContent();
    return a === "Page 5 of 12" && b === "Page 3 of 12" && c === "Page 12 of 12" && nextDisabled && current === "12"
      ? true : `${a} | ${b} | ${c} | nextDisabled=${nextDisabled} current=${current}`;
  });
  await check("pagination: uncontrolled outlined bar moves its own page", async () => {
    const outlined = page.locator(".demo__row", { hasText: "outlined" }).locator(".skeuo-pagination");
    await outlined.locator('[aria-label="Go to page 6"]').click();
    await app.settle();
    return expect(await outlined.locator('[aria-current=page]').textContent(), "6");
  });

  // --------------------------------------------------------------- Stepper
  const stepper = page.locator(".skeuo-stepper").first();
  const activeStep = () => stepper.locator(".skeuo-step--active .skeuo-step-label__label").textContent();
  await check("stepper: Next / Back / Reset", async () => {
    await page.click('.skeuo-btn:has-text("Next") >> nth=0');
    await app.settle();
    const a = await activeStep();
    const completed = await stepper.locator(".skeuo-step--completed").count();
    await page.click('.skeuo-btn:has-text("Back") >> nth=0');
    await app.settle();
    const b = await activeStep();
    await page.click('.skeuo-btn:has-text("Next") >> nth=0');
    await page.click('.skeuo-btn:has-text("Next") >> nth=0');
    await app.settle();
    const label = await page.textContent('.skeuo-btn:has-text("Finish")');
    await page.click('.skeuo-btn:has-text("Finish")');
    await app.settle();
    const done = await stepper.locator(".skeuo-step--completed").count();
    await page.click('.skeuo-btn:has-text("Reset")');
    await app.settle();
    const c = await activeStep();
    return a === "Mix & master" && completed === 2 && b === "Record voice" && label === "Finish" && done === 4 &&
      c === "Prepare script" ? true : `${a} ${completed} ${b} ${label} ${done} ${c}`;
  });
  await check("non-linear stepper: StepButton jumps to a step", async () => {
    const nl = page.locator(".skeuo-stepper.skeuo-color-success");
    await nl.locator('.skeuo-step-button:has-text("Record voice")').click();
    await app.settle();
    return expect(await nl.locator('[aria-current=step]').textContent().then((t) => t.includes("Record voice")), true);
  });
  await check("mobile stepper: Next / Back", async () => {
    const dots = page.locator(".skeuo-mobile-stepper--dots");
    await dots.locator('button:has-text("Next")').click();
    await app.settle();
    const a = await dots.locator(".skeuo-mobile-stepper__dots").getAttribute("aria-label");
    const text = await page.textContent(".skeuo-mobile-stepper__text");
    await dots.locator('button:has-text("Back")').click();
    await dots.locator('button:has-text("Back")').click();
    await app.settle();
    const b = await dots.locator(".skeuo-mobile-stepper__dots").getAttribute("aria-label");
    return a === "Step 4 of 5" && text === "4 / 5" && b === "Step 2 of 5" ? true : `${a} ${text} ${b}`;
  });

  // ------------------------------------------------------ BottomNavigation
  await check("bottom navigation: click selects", async () => {
    const bar = page.locator(".skeuo-bottom-nav").first();
    await bar.locator('button:has-text("Library")').click();
    await app.settle();
    return expect(await bar.locator("[aria-current=page]").textContent(), "Library");
  });
  await check("bottom navigation (labels on selected only): label follows the selection", async () => {
    const bar = page.locator(".skeuo-bottom-nav.skeuo-color-error");
    await bar.locator('[aria-label="Nearby"]').click();
    await app.settle();
    const labels = await bar.locator(".skeuo-bottom-nav__label").allTextContents();
    return expect(labels.join(","), "Nearby");
  });

  // ------------------------------------------------------------- SpeedDial
  const dial = page.locator(".skeuo-speed-dial", { has: page.locator('[aria-label="Clip actions"]') });
  await check("speed dial opens and closes on click", async () => {
    const fab = dial.locator(".skeuo-speed-dial__fab");
    await fab.click();
    await app.settle();
    const open = await fab.getAttribute("aria-expanded");
    const cls = await dial.getAttribute("class");
    const tab = await dial.locator('[aria-label="Rename"]').getAttribute("tabindex");
    await fab.click();
    await app.settle();
    const closed = await fab.getAttribute("aria-expanded");
    return open === "true" && cls.includes("--open") && tab === "0" && closed === "false"
      ? true : `open=${open} tab=${tab} closed=${closed}`;
  });
  await check("speed dial closes on Escape", async () => {
    const fab = dial.locator(".skeuo-speed-dial__fab");
    await fab.click();
    await app.settle();
    await key("Escape");
    return expect(await fab.getAttribute("aria-expanded"), "false");
  });

  if (shots) await page.screenshot({ path: `${shots}/live-navigation.png`, fullPage: true });
  console.log(results.join("\n"));
  console.log(`${app.exchanges} messages exchanged; ${app.errors.length ? "page errors: " + app.errors.join(" | ") : "no page errors"}`);
  await app.close();
  process.exit(results.some((r) => r.startsWith("FAIL")) || app.errors.length ? 1 : 0);
})();
