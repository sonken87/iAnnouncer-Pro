// Interaction test for the Data display page of the C++ gallery:
//   node live-data_display.js path/to/gallery [screenshot-dir]
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
  const expect = (cond, detail) => (cond ? true : detail);
  const demo = (title) => page.locator(`section.demo:has(> h2:text-is("${title}"))`);

  await page.click('.gallery__nav-item:has-text("Data display")');
  await app.settle();

  // ------------------------------------------------------------ Chip
  const chipDemo = demo("Chip");
  await check("clickable chip selects on click", async () => {
    const chip = chipDemo.locator('.skeuo-chip:has(.skeuo-chip__label:text-is("Recorded"))');
    await chip.locator(".skeuo-chip__action").click();
    await app.settle();
    const cls = await chip.getAttribute("class");
    const live = await chipDemo.locator('.skeuo-chip:has(.skeuo-chip__label:text-is("Live"))').getAttribute("class");
    return expect(cls.includes("skeuo-key--filled skeuo-key--pressable") && cls.includes("skeuo-color-primary") &&
                  live.includes("skeuo-chip--outlined"), `recorded=${cls} live=${live}`);
  });
  await check("chip delete removes the tag", async () => {
    await chipDemo.locator('[aria-label="Delete Weather"]').click();
    await app.settle();
    const labels = await chipDemo.locator(".skeuo-chip--deletable.skeuo-color-info .skeuo-chip__label").allTextContents();
    return expect(labels.join(",") === "News,Traffic,Sports,Music", labels.join(","));
  });
  await check("deleting every tag shows Reset, which restores them", async () => {
    for (const t of ["News", "Traffic", "Sports", "Music"]) {
      await chipDemo.locator(`[aria-label="Delete ${t}"]`).click();
      await app.settle(80);
    }
    await app.settle();
    const empty = await chipDemo.locator(".skeuo-chip--deletable.skeuo-color-info").count();
    await chipDemo.locator('.skeuo-btn:has-text("Reset")').click();
    await app.settle();
    const after = await chipDemo.locator(".skeuo-chip--deletable.skeuo-color-info").count();
    return expect(empty === 0 && after === 5, `empty=${empty} after=${after}`);
  });

  // ------------------------------------------------------------ List
  const listDemo = demo("List");
  await check("list rows are selectable", async () => {
    await listDemo.locator('.skeuo-list-item-button:has-text("Schedule")').click();
    await app.settle();
    const current = await listDemo.locator('.skeuo-list-item-button[aria-current="true"]').allTextContents();
    return expect(current.length === 2 && current[0] === "Schedule" && current[1] === "Monitor on", current.join(","));
  });
  await check("dense list selection drives the badge LED", async () => {
    await listDemo.locator('.skeuo-list-item-button:has-text("Muted")').click();
    await app.settle();
    const muted = await listDemo.locator('.skeuo-list-item-button:has-text("Muted")').getAttribute("aria-current");
    const led = await listDemo.locator(".skeuo-list-item__secondary-action .skeuo-badge__badge").getAttribute("class");
    return expect(muted === "true" && led.includes("--invisible"), `muted=${muted} led=${led}`);
  });
  await check("disabled list row stays disabled", async () =>
    listDemo.locator('.skeuo-list-item-button:has-text("Settings (disabled)")').isDisabled());

  // ------------------------------------------------------------ Table
  const table = page.locator('table[aria-label="Clips"]');
  const firstClip = () => table.locator("tbody tr >> nth=0 >> th").textContent();
  await check("table starts sorted by plays, descending", async () => {
    const sort = await table.locator('th[aria-sort]').textContent();
    return expect(sort === "Plays" && (await firstClip()) === "Station ID", `${sort} ${await firstClip()}`);
  });
  await check("sort label activates a column ascending", async () => {
    await table.locator('.skeuo-table-sort-label:has-text("Clip")').click();
    await app.settle();
    const th = table.locator('th:has(.skeuo-table-sort-label:has-text("Clip"))');
    const label = await th.locator(".skeuo-table-sort-label").getAttribute("class");
    return expect((await th.getAttribute("aria-sort")) === "ascending" && label.includes("--active") &&
                  label.includes("--asc") && (await firstClip()) === "News intro", `${label} ${await firstClip()}`);
  });
  await check("sort label toggles direction", async () => {
    await table.locator('.skeuo-table-sort-label:has-text("Clip")').click();
    await app.settle();
    const th = table.locator('th:has(.skeuo-table-sort-label:has-text("Clip"))');
    const label = await th.locator(".skeuo-table-sort-label").getAttribute("class");
    return expect((await th.getAttribute("aria-sort")) === "descending" && label.includes("--desc") &&
                  (await firstClip()) === "Weather bed" && (await table.locator("th[aria-sort]").count()) === 1,
                  `${label} ${await firstClip()}`);
  });
  await check("table rows select and deselect on click", async () => {
    const row = table.locator('tbody tr:has(th:text-is("Station ID"))');
    await row.click();
    await app.settle();
    const on = await row.getAttribute("aria-selected");
    const others = await table.locator('tbody tr[aria-selected="true"]').count();
    await row.click();
    await app.settle();
    const off = await row.getAttribute("aria-selected");
    return expect(on === "true" && others === 1 && off === null, `on=${on} others=${others} off=${off}`);
  });

  // ------------------------------------------------------------ Badge
  const badgeRow = demo("Badge").locator('.demo__row:has(.demo__row-label:text-is("interactive"))');
  const led = badgeRow.locator(".skeuo-badge__badge");
  await check("badge increments and caps at max", async () => {
    const start = await led.textContent();
    await badgeRow.locator('.skeuo-btn:has-text("+")').click();
    await app.settle();
    const five = await led.textContent();
    for (let i = 0; i < 5; i++) await badgeRow.locator('.skeuo-btn:has-text("+")').click();
    await app.settle();
    const capped = await led.textContent();
    await badgeRow.locator('.skeuo-btn:has-text("−")').click();
    await app.settle();
    return expect(start === "4" && five === "5" && capped === "9+" && (await led.textContent()) === "9",
                  `${start} ${five} ${capped} ${await led.textContent()}`);
  });
  await check("badge hides and shows", async () => {
    await badgeRow.locator('.skeuo-btn:has-text("Hide badge")').click();
    await app.settle();
    const hidden = (await led.getAttribute("class")).includes("--invisible") && (await led.getAttribute("aria-hidden")) === "true";
    await badgeRow.locator('.skeuo-btn:has-text("Show badge")').click();
    await app.settle();
    const shown = !(await led.getAttribute("class")).includes("--invisible");
    return expect(hidden && shown, `hidden=${hidden} shown=${shown}`);
  });

  // ------------------------------------------------------------ Avatar
  const groupRow = demo("Avatar").locator('.demo__row:has(.demo__row-label:text-is("AvatarGroup"))');
  const lastGroup = groupRow.locator(".skeuo-avatar-group >> nth=2");
  await check("avatar group shows a surplus cap that tracks Add / Remove", async () => {
    const before = await lastGroup.locator(".skeuo-avatar--surplus").textContent();
    const first = await groupRow.locator(".skeuo-avatar-group >> nth=0 >> .skeuo-avatar--surplus").textContent();
    const second = await groupRow.locator(".skeuo-avatar-group >> nth=1 >> .skeuo-avatar--surplus").textContent();
    await groupRow.locator('.skeuo-btn:has-text("Add")').click();
    await app.settle();
    const added = await lastGroup.locator(".skeuo-avatar--surplus").textContent();
    for (let i = 0; i < 3; i++) await groupRow.locator('.skeuo-btn:has-text("Remove")').click();
    await app.settle();
    const items = await lastGroup.locator(".skeuo-avatar-group__item").count();
    const surplus = await lastGroup.locator(".skeuo-avatar--surplus").count();
    const overlap = await lastGroup.evaluate((el) => getComputedStyle(el.children[1]).marginLeft);
    return expect(first === "+3" && second === "+20" && before === "+2" && added === "+3" && items === 5 &&
                  surplus === 0 && overlap === "-8px",
                  `${first} ${second} ${before} ${added} items=${items} surplus=${surplus} overlap=${overlap}`);
  });
  await check("avatar image error falls back to the alt letter", async () => {
    const dawn = demo("Avatar").locator('.demo__row:has(.demo__row-label:text-is("photos")) .skeuo-avatar >> nth=0');
    await dawn.locator("img").evaluate((img) => { img.src = "data:image/png;base64,broken"; });
    await app.settle(400);
    const cls = await dawn.getAttribute("class");
    const txt = await dawn.textContent();
    const imgs = await dawn.locator("img").count();
    return expect(cls.includes("skeuo-avatar--letter") && txt === "D" && imgs === 0, `${cls} "${txt}" imgs=${imgs}`);
  });

  // ------------------------------------------------------------ Tooltip
  const tipDemo = demo("Tooltip");
  const hoverTip = tipDemo.locator('.skeuo-tooltip:has(.skeuo-btn:has-text("Hover me"))');
  const popper = hoverTip.locator(".skeuo-tooltip__popper");
  const shown = () => popper.evaluate((el) => {
    const s = getComputedStyle(el);
    return s.display !== "none" && s.visibility === "visible" && el.getBoundingClientRect().height > 0;
  });
  await check("tooltip is hidden until hovered, then appears (computed style)", async () => {
    await page.mouse.move(5, 5);
    const before = await shown();
    await hoverTip.locator(".skeuo-btn").hover();
    await app.settle(300);
    const during = await shown();
    const text = await popper.textContent();
    await page.mouse.move(5, 5);
    await app.settle(200);
    const after = await shown();
    return expect(!before && during && !after && text === "Delete clip", `before=${before} during=${during} after=${after}`);
  });
  await check("forced-open tooltips are visible without hover", async () => {
    const all = await tipDemo.locator(".skeuo-tooltip--open > .skeuo-tooltip__popper").evaluateAll((els) =>
      els.map((el) => getComputedStyle(el).display !== "none"));
    return expect(all.length === 5 && all.every(Boolean), JSON.stringify(all));
  });
  await check("Escape dismisses a hovered tooltip until the pointer leaves", async () => {
    await hoverTip.locator(".skeuo-btn").hover();
    await hoverTip.locator(".skeuo-btn").focus();
    await app.settle(200);
    const before = await shown();
    await page.keyboard.press("Escape");
    await app.settle();
    const dismissed = !(await shown()) && (await hoverTip.getAttribute("class")).includes("--closed");
    await page.mouse.move(5, 5);
    await app.settle();
    const restored = (await hoverTip.getAttribute("class")).includes("--auto");
    await hoverTip.locator(".skeuo-btn").hover();
    await app.settle(200);
    const again = await shown();
    return expect(before && dismissed && restored && again, `before=${before} dismissed=${dismissed} restored=${restored} again=${again}`);
  });
  await check("controlled tooltip follows its chip", async () => {
    // The previous check leaves the pointer on "Hover me", whose interactive
    // bubble sits over this row (and focus keeps it up via :focus-visible).
    await page.mouse.move(5, 5);
    await page.evaluate(() => document.activeElement && document.activeElement.blur());
    await app.settle();
    const tip = tipDemo.locator('.demo__row:has(.demo__row-label:text-is("controlled")) .skeuo-tooltip');
    await tip.locator(".skeuo-chip__action").click();
    await app.settle();
    const closed = (await tip.getAttribute("class")).includes("--closed") &&
      (await tip.locator(".skeuo-chip__label").textContent()) === "open: Some(false)";
    await tip.locator(".skeuo-chip__action").click();
    await app.settle();
    const open = (await tip.getAttribute("class")).includes("--open");
    return expect(closed && open, `closed=${closed} open=${open}`);
  });
  await check("tooltip shows on keyboard focus", async () => {
    const tip = tipDemo.locator('.skeuo-tooltip:has-text("Tab to me")');
    await tipDemo.locator('.skeuo-btn:has-text("Hover me")').focus();
    await page.mouse.move(5, 5);
    await page.keyboard.press("Tab");
    await app.settle(200);
    const vis = await tip.locator(".skeuo-tooltip__popper").evaluate((el) => getComputedStyle(el).display !== "none");
    return expect(vis, "not shown on focus");
  });

  if (shots) await page.screenshot({ path: `${shots}/live-data-display.png`, fullPage: true });
  console.log(results.join("\n"));
  console.log(`${app.exchanges} messages exchanged; ${app.errors.length ? "page errors: " + app.errors.join(" | ") : "no page errors"}`);
  await app.close();
  process.exit(results.some((r) => r.startsWith("FAIL")) || app.errors.length ? 1 : 0);
})();
