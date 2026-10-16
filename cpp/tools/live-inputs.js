// Interaction test for the Inputs page of the C++ gallery:
//   node live-inputs.js path/to/gallery [screenshot-dir]
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
  const text = (sel) => page.textContent(sel);
  // The demo plate (section) with this title.
  const demo = (title) => `section.demo:has(> h2:text-is("${title}"))`;
  // The field (Select / Autocomplete) whose label is `label`.
  const field = (label) => `.skeuo-field:has(> label:text-is("${label}"))`;

  await page.click('.gallery__nav-item:has-text("Inputs")');
  await app.settle();

  // ---------------------------------------------------------------- Button
  await check("button loading: Save (click) turns into a spinner, Reset restores it", async () => {
    await page.click('button:has-text("Save (click)")');
    await app.settle();
    const busy = await page.getAttribute('button:has-text("Save (click)")', "aria-busy");
    await page.click('button:has-text("Reset")');
    await app.settle();
    const after = await page.getAttribute('button:has-text("Save (click)")', "aria-busy");
    return busy === "true" && after === null ? true : `busy=${busy} after=${after}`;
  });

  // ---------------------------------------------------------------- ButtonGroup
  await check("ButtonGroup: clicking Ch C updates the caption", async () => {
    await page.click(`${demo("ButtonGroup")} button:has-text("Ch C")`);
    await app.settle();
    return (await page.isVisible("text=full_width · on channel C")) || "caption not updated";
  });
  await check("ButtonGroup: disabled group disables its buttons", async () =>
    (await page.isDisabled(`${demo("ButtonGroup")} button:has-text("Disabled")`)) || "not disabled");

  // ---------------------------------------------------------------- IconButton / Fab
  await check("IconButton: heart toggles", async () => {
    await page.click('button[aria-label="Like"]');
    await app.settle();
    const pressed = await page.getAttribute('button[aria-label="Like"]', "aria-pressed");
    const cap = await page.isVisible("text=Liked — click the heart again");
    return pressed === "true" && cap ? true : `pressed=${pressed} caption=${cap}`;
  });
  await check("Fab: Add counts clicks", async () => {
    await page.click(`${demo("Fab")} button[aria-label="Add"] >> nth=0`);
    await page.click(`${demo("Fab")} button[aria-label="Add"] >> nth=0`);
    await app.settle();
    return (await page.isVisible("text=Added 2")) || "count not 2";
  });

  // ---------------------------------------------------------------- ToggleButton
  await check("ToggleButtonGroup exclusive: Center selects, Left releases", async () => {
    await page.click('button[aria-label="Center"]');
    await app.settle();
    const c = await page.getAttribute('button[aria-label="Center"]', "aria-pressed");
    const l = await page.getAttribute('button[aria-label="Left"]', "aria-pressed");
    const cap = await page.isVisible("text=align = center");
    return c === "true" && l === "false" && cap ? true : `center=${c} left=${l} caption=${cap}`;
  });
  await check("ToggleButtonGroup exclusive: pressing the selected key releases it", async () => {
    await page.click('button[aria-label="Center"]');
    await app.settle();
    return (await page.isVisible("text=align = none")) || "not released";
  });
  await check("ToggleButtonGroup multiple: underline adds, bold removes", async () => {
    await page.click('button[value="underline"]');
    await page.click('button[value="bold"]');
    await app.settle();
    return (await page.isVisible("text=formats = italic + underline")) || (await text(`${demo("ToggleButton")} .demo__row >> nth=1`));
  });
  await check("ToggleButtonGroup uncontrolled: Week latches down", async () => {
    await page.click('button[value="week"]');
    await app.settle();
    const cls = await page.getAttribute('button[value="week"]', "class");
    const day = await page.getAttribute('button[value="day"]', "aria-pressed");
    return cls.includes("skeuo-toggle--selected") && day === "false" ? true : cls;
  });
  await check("ToggleButton standalone toggles", async () => {
    await page.click('button[aria-label="Standalone"]');
    await app.settle();
    return (await page.isVisible("text=standalone: false")) || "still true";
  });

  // ---------------------------------------------------------------- TextareaAutosize
  await check("TextareaAutosize grows while typing", async () => {
    const ta = 'textarea[aria-label="Notes"]';
    const h0 = (await page.locator(ta).boundingBox()).height;
    await page.click(ta);
    await page.keyboard.type("one", { delay: 5 });
    for (const line of ["two", "three", "four"]) {
      await page.keyboard.press("Enter");
      await page.keyboard.type(line, { delay: 5 });
    }
    // Every keystroke round-trips to C++; wait for the last one to land.
    await page.waitForFunction((sel) => document.querySelector(sel).getAttribute("rows") === "4", ta, { timeout: 3000 }).catch(() => {});
    const rows = await page.getAttribute(ta, "rows");
    const h1 = (await page.locator(ta).boundingBox()).height;
    const v = await page.inputValue(ta);
    return rows === "4" && h1 > h0 + 20 && v === "one\ntwo\nthree\nfour" ? true : `rows=${rows} h ${h0}->${h1} v=${JSON.stringify(v)}`;
  });
  await check("TextareaAutosize controlled: line count caption follows", async () => {
    const ta = 'textarea[aria-label="Controlled notes"]';
    await page.click(ta);
    await page.keyboard.press("End");
    await page.keyboard.press("Control+End");
    await page.keyboard.press("Enter");
    await page.keyboard.type("Line three", { delay: 5 });
    // Every keystroke round-trips to C++; wait for the last one to land.
    await page.waitForFunction((sel) => document.querySelector(sel).getAttribute("rows") === "4", ta, { timeout: 3000 }).catch(() => {});
    const rows = await page.getAttribute(ta, "rows");
    return (await page.isVisible("text=3 line(s)")) && rows === "3" ? true : `rows=${rows}`;
  });

  // ---------------------------------------------------------------- Select
  const station = field("Station");
  await check("Select: click opens the list", async () => {
    await page.click(`${station} [role=combobox]`);
    await app.settle();
    const expanded = await page.getAttribute(`${station} [role=combobox]`, "aria-expanded");
    return expanded === "true" && (await page.isVisible(`${station} [role=listbox]`)) ? true : `expanded=${expanded}`;
  });
  await check("Select: choose with the mouse", async () => {
    await page.click(`${station} [role=option]:has-text("810 AM")`);
    await app.settle();
    const value = await text(`${station} .skeuo-select__value`);
    const open = await page.isVisible(`${station} [role=listbox]`);
    return value === "810 AM" && !open && (await page.isVisible("text=value = am810")) ? true : `value=${value} open=${open}`;
  });
  await check("Select: keyboard — focus, ArrowDown opens, ArrowUp moves, Enter chooses", async () => {
    await page.focus(`${station} [role=combobox]`);
    await page.keyboard.press("ArrowDown");
    await app.settle();
    const open = await page.isVisible(`${station} [role=listbox]`);
    const active0 = await text(`${station} .skeuo-select__option--active`);
    await page.keyboard.press("ArrowUp");
    await app.settle();
    const active1 = await text(`${station} .skeuo-select__option--active`);
    await page.keyboard.press("Enter");
    await app.settle();
    const value = await text(`${station} .skeuo-select__value`);
    const closed = !(await page.isVisible(`${station} [role=listbox]`));
    const focusKept = await page.evaluate(() => document.activeElement.getAttribute("role"));
    return open && active0 === "810 AM" && active1 === "95.4 FM" && value === "95.4 FM" && closed && focusKept === "combobox"
      ? true : `open=${open} active ${active0}->${active1} value=${value} closed=${closed} focus=${focusKept}`;
  });
  await check("Select: ArrowDown skips the disabled row, End/Home, Escape closes without choosing", async () => {
    await page.keyboard.press("Enter");
    await app.settle();
    await page.keyboard.press("End");
    await app.settle();
    const end = await text(`${station} .skeuo-select__option--active`);
    await page.keyboard.press("Home");
    await page.keyboard.press("ArrowDown");
    await app.settle();
    const second = await text(`${station} .skeuo-select__option--active`);
    await page.keyboard.press("Escape");
    await app.settle();
    const closed = !(await page.isVisible(`${station} [role=listbox]`));
    const value = await text(`${station} .skeuo-select__value`);
    return end === "DAB+ Digital" && second === "95.4 FM" && closed && value === "95.4 FM" ? true : `end=${end} second=${second} closed=${closed} value=${value}`;
  });
  await check("Select: Tab away closes the list", async () => {
    await page.keyboard.press("Space");
    await app.settle();
    const open = await page.isVisible(`${station} [role=listbox]`);
    await page.keyboard.press("Tab");
    await app.settle();
    return open && !(await page.isVisible(`${station} [role=listbox]`)) ? true : `open=${open}`;
  });
  await check("Select: clicking elsewhere closes the list", async () => {
    await page.click(`${station} [role=combobox]`);
    await app.settle();
    const open = await page.isVisible(`${station} [role=listbox]`);
    await page.click(`${demo("Select")} h2`);
    await app.settle();
    return open && !(await page.isVisible(`${station} [role=listbox]`)) ? true : `open=${open}`;
  });
  await check("Select multiple: rows toggle and the list stays open", async () => {
    const days = field("Days");
    await page.click(`${days} [role=combobox]`);
    await app.settle();
    await page.click(`${days} [role=option]:has-text("Friday")`);
    await app.settle();
    await page.click(`${days} [role=option]:has-text("Monday")`);
    await app.settle();
    const value = await text(`${days} .skeuo-select__value`);
    const open = await page.isVisible(`${days} [role=listbox]`);
    await page.keyboard.press("Escape");
    await app.settle();
    const closed = !(await page.isVisible(`${days} [role=listbox]`));
    return value === "Wednesday, Friday" && open && closed ? true : `value=${value} open=${open} closed=${closed}`;
  });
  await check("NativeSelect reports its value", async () => {
    await page.selectOption("select.skeuo-native-select__input", "30");
    await app.settle();
    return (await page.isVisible("text=platform list · 30 min")) || "caption not updated";
  });

  // ---------------------------------------------------------------- Autocomplete
  const programme = field("Programme");
  const pinput = `${programme} input`;
  await check("Autocomplete: typing filters the options", async () => {
    await page.click(pinput);
    await page.fill(pinput, "");
    await page.keyboard.type("ea", { delay: 10 });
    await app.settle(300);
    const opts = await page.locator(`${programme} [role=option]`).allTextContents();
    return JSON.stringify(opts) === JSON.stringify(["Breakfast Show", "Weather Watch"]) ? true : JSON.stringify(opts);
  });
  await check("Autocomplete: choose with the mouse", async () => {
    await page.click(`${programme} [role=option]:has-text("Weather Watch")`);
    await app.settle();
    const v = await page.inputValue(pinput);
    return v === "Weather Watch" && (await page.isVisible("text=value = Weather Watch")) && !(await page.isVisible(`${programme} [role=listbox]`)) ? true : `v=${v}`;
  });
  await check("Autocomplete: keyboard — type, ArrowDown, Enter", async () => {
    await page.click(pinput);
    await page.fill(pinput, "");
    await page.keyboard.type("news", { delay: 10 });
    await app.settle(300);
    await page.keyboard.press("ArrowDown");
    await app.settle();
    await page.keyboard.press("Enter");
    await app.settle();
    const v = await page.inputValue(pinput);
    return v === "Morning News" && (await page.isVisible("text=value = Morning News")) ? true : `v=${v}`;
  });
  await check("Autocomplete: Escape closes; leaving restores the value's text", async () => {
    await page.click(pinput);
    await page.keyboard.press("End");
    await page.keyboard.type("zz", { delay: 10 });
    await app.settle(300);
    const empty = await page.isVisible(`${programme} .skeuo-select__empty`);
    await page.keyboard.press("Escape");
    await app.settle();
    const closed = !(await page.isVisible(`${programme} [role=listbox]`));
    await page.click(`${demo("Autocomplete")} h2`);
    await app.settle();
    const v = await page.inputValue(pinput);
    return empty && closed && v === "Morning News" ? true : `empty=${empty} closed=${closed} v=${v}`;
  });
  await check("Autocomplete: clear button empties the value", async () => {
    await page.click(pinput);
    await page.click(`${programme} button[aria-label="Clear"]`);
    await app.settle();
    const v = await page.inputValue(pinput);
    return v === "" && (await page.isVisible("text=value = none")) ? true : `v=${v}`;
  });
  await check("Autocomplete free solo: Enter keeps typed text", async () => {
    const fs = `${field("Free solo")} input`;
    await page.click(fs);
    await page.keyboard.type("My own show", { delay: 5 });
    await app.settle(300);
    await page.keyboard.press("Enter");
    await app.settle();
    await page.click(`${demo("Autocomplete")} h2`);
    await app.settle();
    return (await page.inputValue(fs)) === "My own show" || (await page.inputValue(fs));
  });
  const playlist = field("Playlist (multiple)");
  const pills = () => page.locator(`${playlist} .skeuo-autocomplete__tag-label`).allTextContents();
  await check("Autocomplete multiple: pick adds a pill, list stays open", async () => {
    await page.click(`${playlist} input`);
    await page.keyboard.type("jazz", { delay: 10 });
    await app.settle(300);
    await page.click(`${playlist} [role=option]:has-text("Late Night Jazz")`);
    await app.settle(400);
    const p = await pills();
    const open = await page.isVisible(`${playlist} [role=listbox]`);
    const v = await page.inputValue(`${playlist} input`);
    // Focus returns to the text field (the pills before it changed).
    const focus = await page.evaluate(() => document.activeElement.classList.contains("skeuo-autocomplete__input"));
    return JSON.stringify(p) === JSON.stringify(["Morning News", "The Chart", "Late Night Jazz"]) && open && v === "" && focus ? true : `pills=${JSON.stringify(p)} open=${open} v=${v} focus=${focus}`;
  });
  await check("Autocomplete multiple: keyboard still works after a mouse pick", async () => {
    await page.keyboard.press("ArrowDown");
    await app.settle();
    await page.keyboard.press("Enter");
    await app.settle(400);
    const p = await pills();
    return p.length === 4 ? true : JSON.stringify(p);
  });
  await check("Autocomplete multiple: removing a pill with its ×", async () => {
    await page.click(`${playlist} button[aria-label="Remove The Chart"]`);
    await app.settle(400);
    const p = await pills();
    const focus = await page.evaluate(() => document.activeElement.classList.contains("skeuo-autocomplete__input"));
    return !p.includes("The Chart") && (await page.isVisible("text=3 selected")) && focus ? true : `${JSON.stringify(p)} focus=${focus}`;
  });
  await check("Autocomplete multiple: Backspace in the empty field removes the last pill", async () => {
    const before = await pills();
    await page.keyboard.press("Backspace");
    await app.settle(400);
    const p = await pills();
    return JSON.stringify(p) === JSON.stringify(before.slice(0, -1)) ? true : JSON.stringify(p);
  });
  await check("Autocomplete multiple: Backspace still deletes typed text", async () => {
    await page.keyboard.type("ab", { delay: 10 });
    await app.settle(300);
    await page.keyboard.press("Backspace");
    await app.settle(300);
    const v = await page.inputValue(`${playlist} input`);
    const p = await pills();
    return v === "a" && p.length === 2 ? true : `v=${v} pills=${JSON.stringify(p)}`;
  });
  await check("Autocomplete: clicking elsewhere closes the list", async () => {
    await page.click(`${demo("Autocomplete")} h2`);
    await app.settle();
    return !(await page.isVisible(`${playlist} [role=listbox]`)) || "still open";
  });

  // ---------------------------------------------------------------- TransferList
  const tl = demo("TransferList");
  await check("TransferList: move the checked rows right", async () => {
    await page.click(`${tl} button[aria-label="Move selected right"]`);
    await app.settle();
    const cap = await text(`${tl} p.skeuo-caption`);
    return cap === "Running order: Top of hour → News → Station ID → Weather" || cap;
  });
  await check("TransferList: check a row by its label, move it, move all back", async () => {
    await page.click(`${tl} .skeuo-transfer__row-label:has-text("Traffic")`);
    await app.settle();
    const sel = await page.isVisible(`${tl} >> text=1/3 selected`);
    await page.click(`${tl} button[aria-label="Move selected right"]`);
    await app.settle();
    const cap = await text(`${tl} p.skeuo-caption`);
    await page.click(`${tl} button[aria-label="Move all left"]`);
    await app.settle();
    const after = await text(`${tl} p.skeuo-caption`);
    const disabled = await page.isDisabled(`${tl} button[aria-label="Move all left"]`);
    return sel && cap.endsWith("→ Traffic") && after === "Running order: " && disabled ? true : `sel=${sel} cap=${cap} after=${after}`;
  });
  await check("TransferList: select-all checkbox", async () => {
    await page.click(`${tl} input[aria-label="Select all Library"]`);
    await app.settle();
    const n = await page.locator(`${tl} .skeuo-transfer__row--checked`).count();
    return n === 7 || `checked rows=${n}`;
  });

  if (shots) await page.screenshot({ path: `${shots}/live-inputs.png`, fullPage: true });
  console.log(results.join("\n"));
  console.log(`${app.exchanges} messages exchanged; ${app.errors.length ? "page errors: " + app.errors.join(" | ") : "no page errors"}`);
  await app.close();
  process.exit(results.some((r) => r.startsWith("FAIL")) ? 1 : 0);
})();
