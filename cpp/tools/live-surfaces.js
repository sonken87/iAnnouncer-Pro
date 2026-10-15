// Interaction test for the Surfaces page: `node live-surfaces.js path/to/gallery [screenshot-dir]`
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
  const summary = (title) => page.locator(".skeuo-accordion-summary", { hasText: title });
  const expanded = async (title) => (await summary(title).getAttribute("aria-expanded")) === "true";
  const states = async (titles) => {
    const out = [];
    for (const t of titles) out.push(await expanded(t));
    return out.join(",");
  };
  const exclusive = ["Microphones", "Playout", "Streaming"];

  await check("navigate to Surfaces & layout", async () => {
    await page.click('.gallery__nav-item:has-text("Surfaces")');
    await app.settle();
    return (await page.textContent(".gallery__main h1")) === "Surfaces & layout";
  });

  // ------------------------------------------------------------ Accordion
  await check("exclusive accordion starts with Playout open", async () =>
    (await states(exclusive)) === "false,true,false" || await states(exclusive));
  await check("opening Microphones closes Playout (controlled, exclusive)", async () => {
    await summary("Microphones").click();
    await app.settle();
    const s = await states(exclusive);
    const open = await page.locator(".skeuo-accordion--expanded", { hasText: "Microphones" }).count();
    const region = await page.getAttribute(`#${await summary("Microphones").getAttribute("aria-controls")}`, "class");
    return s === "true,false,false" && open === 1 && region.includes("--open") ? true : `${s} region=${region}`;
  });
  await check("opening Streaming closes Microphones", async () => {
    await summary("Streaming").click();
    await app.settle();
    const s = await states(exclusive);
    return s === "false,false,true" ? true : s;
  });
  await check("clicking the open section collapses it (none open)", async () => {
    await summary("Streaming").click();
    await app.settle();
    const s = await states(exclusive);
    return s === "false,false,false" ? true : s;
  });
  await check("disabled section is disabled and stays closed", async () => {
    const disabled = await summary("Disabled section").isDisabled();
    await summary("Disabled section").click({ force: true });
    await app.settle();
    return disabled && !(await expanded("Disabled section"));
  });
  await check("uncontrolled: default-expanded section collapses and reopens", async () => {
    const t = "Square, no gutters";
    const a = await expanded(t);
    await summary(t).click();
    await app.settle();
    const b = await expanded(t);
    await summary(t).click();
    await app.settle();
    const c = await expanded(t);
    return a && !b && c ? true : `${a},${b},${c}`;
  });
  await check("uncontrolled sections are independent", async () => {
    await summary("Collapsed").click();
    await summary("With an icon").click();
    await app.settle();
    const s = await states(["Square, no gutters", "Collapsed", "With an icon"]);
    const ex = await states(exclusive);
    return s === "true,true,true" && ex === "false,false,false" ? true : `${s} / ${ex}`;
  });

  // ------------------------------------------------------------ Card
  await check("CardActionArea click counter", async () => {
    const area = page.locator(".skeuo-card-action-area");
    await area.click();
    await area.click();
    await area.click();
    await app.settle();
    const t = await area.textContent();
    return t.includes("Pressed 3 times") ? true : t;
  });
  await check("like button toggles color and icon", async () => {
    const like = page.locator('[aria-label="Like"]');
    await like.click();
    await app.settle();
    const cls = await like.getAttribute("class");
    return cls.includes("skeuo-color-error") ? true : cls;
  });

  // ------------------------------------------------------------ AppBar
  await check("app bar buttons are real, clickable keys", async () => {
    const bars = page.locator(".sd-bars .skeuo-appbar");
    if ((await bars.count()) !== 5) return `bars=${await bars.count()}`;
    for (const label of ["Open menu", "Search", "More"]) {
      const b = bars.nth(0).locator(`[aria-label="${label}"]`);
      await b.click();
      if (!(await b.isEnabled())) return `${label} disabled`;
    }
    await bars.nth(1).locator('button:has-text("Go live")').click();
    await app.settle();
    return app.errors.length === 0 ? true : app.errors.join(" | ");
  });
  await check("app bar colors", async () => {
    const cls = await page.locator(".sd-bars .skeuo-appbar").evaluateAll((els) => els.map((e) => e.className));
    return cls[0].includes("skeuo-appbar--primary") && cls[1].includes("skeuo-chassis") &&
      cls[2].includes("skeuo-color-secondary") && cls[3].includes("skeuo-color-success") &&
      cls[4].includes("skeuo-appbar--transparent") ? true : cls.join(" / ");
  });

  // ------------------------------------------------------------ Drawer
  const modal = '.skeuo-drawer--temporary:not(.skeuo-drawer--inline)';
  for (const side of ["Left", "Right", "Top", "Bottom"]) {
    const anchor = side.toLowerCase();
    await check(`drawer opens from the ${anchor} and takes focus`, async () => {
      await page.click(`button:has-text("Open ${side}")`);
      await app.settle(300);
      const cls = await page.getAttribute(modal, "class");
      const focused = await page.evaluate(() => document.activeElement.getAttribute("aria-label"));
      if (shots && side === "Left") await page.screenshot({ path: `${shots}/live-surfaces-drawer.png` });
      return cls.includes(`skeuo-drawer--${anchor}`) && focused === "Navigation" ? true : `${cls} focus=${focused}`;
    });
    if (side === "Left" || side === "Top") {
      await check(`Escape closes the ${anchor} drawer`, async () => {
        await page.keyboard.press("Escape");
        await app.settle(300);
        return (await page.locator(modal).count()) === 0;
      });
    } else {
      await check(`scrim click closes the ${anchor} drawer`, async () => {
        // Click the scrim on the side away from the panel.
        const pos = anchor === "right" ? { x: 40, y: 450 } : { x: 640, y: 60 };
        await page.locator(`${modal} .skeuo-drawer__scrim`).click({ position: pos });
        await app.settle(300);
        return (await page.locator(modal).count()) === 0;
      });
    }
  }
  await check("Close button inside the drawer closes it", async () => {
    await page.click('button:has-text("Open Left")');
    await app.settle(300);
    await page.locator(`${modal} button:has-text("Close")`).click();
    await app.settle(300);
    return (await page.locator(modal).count()) === 0;
  });
  await check("Escape inside the inline preview does not affect it", async () => {
    await page.locator('[aria-label="Preview navigation"] button:has-text("Home")').focus();
    await page.keyboard.press("Escape");
    await app.settle();
    return (await page.locator(".skeuo-drawer--inline").count()) === 1;
  });
  await check("page state survived all interactions", async () => {
    const t = await page.locator(".skeuo-card-action-area").textContent();
    return t.includes("Pressed 3 times") && (await states(["Square, no gutters"])) === "true" ? true : t;
  });

  if (shots) await page.screenshot({ path: `${shots}/live-surfaces.png`, fullPage: true });
  console.log(results.join("\n"));
  console.log(`${app.exchanges} messages exchanged; ${app.errors.length ? "page errors: " + app.errors.join(" | ") : "no page errors"}`);
  await app.close();
  process.exit(results.some((r) => r.startsWith("FAIL")) ? 1 : 0);
})();
