// Interaction checks for the Map page and ScrollArea of the C++ gallery:
// `node live-map.js path/to/gallery [screenshot-dir]`. Map tiles are served
// offline (a plain tile) so the test does not depend on a tile server.
const { launch } = require("./harness");

// A 1x1 light-grey PNG used for every tile.
const TILE = Buffer.from(
  "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAIAAACQd1PeAAAADElEQVR4nGPYvXs3AAQ6AhQrO4N9AAAAAElFTkSuQmCC", "base64");

(async () => {
  const [binary, shots] = process.argv.slice(2);
  let tileRequests = 0;
  const app = await launch(binary, {
    viewport: { width: 1280, height: 1000 },
    setup: (page) => page.route(/\/\d+\/\d+\/\d+\.png/, (route) => {
      tileRequests++;
      route.fulfill({ status: 200, contentType: "image/png", body: TILE });
    }),
  });
  const { page } = app;
  const results = [];
  const check = async (name, fn) => {
    try {
      const r = await fn();
      results.push(`${r === false ? "FAIL" : "ok  "} ${name}${typeof r === "string" ? " — " + r : ""}`);
    } catch (e) {
      results.push(`FAIL ${name} — ${e.message.split("\n")[0].slice(0, 200)}`);
    }
  };
  const settle = (ms = 500) => page.waitForTimeout(ms);

  await page.click('.gallery__nav-item:has-text("Map")');
  const map1 = page.locator(".skeuo-map").first();
  const wps = () => map1.locator(".skeuo-map__waypoint").count();
  const rows = () => page.locator(".skeuo-table tbody tr").count();

  await check("Leaflet maps created in the wells", async () => {
    await page.waitForSelector(".skeuo-map .leaflet-container", { timeout: 10000 });
    return (await page.locator(".skeuo-map .leaflet-container").count()) === 2;
  });
  await settle(800);
  await check("route drawn: 6 waypoints, 5 legs, 5 leg labels", async () => {
    const r = [await wps(), await map1.locator(".skeuo-map__leg").count(), await map1.locator(".skeuo-map__leg-label").count()];
    return r.join() === "6,5,5" ? true : "got " + r.join();
  });
  await check("tiles requested", async () => (tileRequests > 0 ? `${tileRequests} tiles` : false));
  await check("navlog: 5 legs", async () => (await rows()) === 5 ? true : `${await rows()}`);
  await check("C++ re-render keeps the Leaflet map (data-sk-ignore)", async () => {
    await page.click('button:has-text("Reverse")');
    await settle();
    const first = await page.locator(".skeuo-table tbody tr").first().innerText();
    await page.click('button:has-text("Reverse")');
    await settle();
    return (await page.locator(".skeuo-map .leaflet-container").count()) === 2 && first.includes("ZSPD") ? true : first;
  });
  if (shots) await map1.screenshot({ path: `${shots}/cpp-map-plan.png` });
  await check("click map adds WPT1 (C++ state + navlog)", async () => {
    const box = await map1.boundingBox();
    await page.mouse.click(box.x + box.width * 0.85, box.y + box.height * 0.2);
    await settle();
    const n = await wps(), r = await rows(), t = await map1.locator('.skeuo-map__tag:has-text("WPT1")').count();
    return n === 7 && r === 6 && t === 1 ? true : `waypoints=${n} rows=${r} wpt1=${t}`;
  });
  await check("drag a waypoint changes the total", async () => {
    const before = await page.locator(".skeuo-table tfoot").innerText();
    const box = await map1.locator('.skeuo-map__waypoint:has-text("JNN") .skeuo-map__symbol').boundingBox();
    await page.mouse.move(box.x + 12, box.y + 12);
    await page.mouse.down();
    await page.mouse.move(box.x + 80, box.y + 40, { steps: 8 });
    await page.mouse.up();
    await settle();
    const after = await page.locator(".skeuo-table tfoot").innerText();
    return before !== after ? after.replace(/\s+/g, " ") : false;
  });
  await check("double-click removes a waypoint", async () => {
    await map1.locator('.skeuo-map__waypoint:has-text("P42") .skeuo-map__symbol').dblclick();
    await settle();
    return (await map1.locator('.skeuo-map__waypoint:has-text("P42")').count()) === 0 && (await wps()) === 6;
  });
  await check("drag a leg midpoint inserts a waypoint", async () => {
    await map1.hover();
    const box = await map1.locator(".skeuo-map__insert").first().boundingBox();
    await page.mouse.move(box.x + 7, box.y + 7);
    await page.mouse.down();
    await page.mouse.move(box.x + 50, box.y - 30, { steps: 8 });
    await page.mouse.up();
    await settle();
    return (await wps()) === 7 ? true : `${await wps()}`;
  });
  await check("click a waypoint selects it", async () => {
    await map1.locator('.skeuo-map__waypoint:has-text("SSN") .skeuo-map__symbol').click();
    await settle();
    return (await map1.locator('.skeuo-map__waypoint--selected:has-text("SSN")').count()) === 1;
  });
  await check("navlog row click selects the waypoint on the map", async () => {
    await page.locator(".skeuo-table tbody tr").nth(1).click();
    await settle();
    return (await map1.locator(".skeuo-map__waypoint--selected").count()) === 1;
  });
  await check("ground speed field updates the summary", async () => {
    await page.locator('.skeuo-field:has(label:has-text("Ground speed")) input').fill("300");
    await settle();
    return (await page.isVisible("text=/at 300 kt/")) ? true : false;
  });
  await check("zoom keycap zooms in", async () => {
    const z0 = await page.evaluate(() => [...window.__skeuoMap.maps][0].map.getZoom());
    await map1.locator('.skeuo-map__key[title="Zoom in"]').click();
    await settle(700);
    const z1 = await page.evaluate(() => [...window.__skeuoMap.maps][0].map.getZoom());
    return z1 > z0 ? `${z0} -> ${z1}` : false;
  });
  await check("Reset restores the demo route", async () => {
    await page.click('button:has-text("Reset")');
    await settle(700);
    return (await wps()) === 6 && (await rows()) === 5;
  });
  const map2 = page.locator(".skeuo-map").nth(1);
  await check("long-haul map: 2 legs, aircraft, area, KLAX shown", async () => {
    await map2.scrollIntoViewIfNeeded();
    await settle(600);
    const r = [await map2.locator(".skeuo-map__leg").count(), await map2.locator(".skeuo-map__marker--aircraft").count(),
               await map2.locator(".skeuo-map__area").count(), await map2.locator('.skeuo-map__tag:has-text("KLAX")').first().isVisible()];
    return r.join() === "2,1,1,true" ? true : "got " + r.join();
  });
  if (shots) await map2.screenshot({ path: `${shots}/cpp-map-longhaul.png` });
  await check("dark mode dims the tiles", async () => {
    await page.click('[role=tab]:has-text("Dark")');
    await settle();
    const f = await page.evaluate(() => getComputedStyle(document.querySelector(".skeuo-map__tiles")).filter);
    return f.includes("invert");
  });
  await check("ScrollArea scrolls (Surfaces page)", async () => {
    await page.click('.gallery__nav-item:has-text("Surfaces")');
    await settle();
    const area = page.locator(".skeuo-scroll-area").first();
    await area.hover();
    await page.mouse.wheel(0, 200);
    await settle(300);
    const top = await area.evaluate((e) => e.scrollTop);
    return top > 0 ? `scrollTop=${top}` : false;
  });

  console.log(results.join("\n"));
  console.log(`${app.exchanges} messages exchanged; ${app.errors.length ? "page errors: " + app.errors.join(" | ") : "no page errors"}`);
  await app.close();
  process.exit(results.some((r) => r.startsWith("FAIL")) ? 1 : 0);
})();
