// Interaction smoke test for the C++ gallery: `node live-check.js path/to/gallery [screenshot-dir]`
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

  await check("button clicks reach C++ and re-render", async () => {
    await page.click('.skeuo-btn:has-text("primary") >> nth=0');
    await page.click('.skeuo-btn:has-text("primary") >> nth=0');
    await app.settle();
    return (await page.textContent("text=/Clicked \\d+ times/")).includes("2");
  });
  await check("typing keeps every character and updates helper", async () => {
    await page.click('input[placeholder="Your name"]');
    await page.keyboard.type("Announcer Ken", { delay: 5 });
    await app.settle(400);
    const v = await page.inputValue('input[placeholder="Your name"]');
    const helper = await page.isVisible("text=Hello, Announcer Ken");
    return v === "Announcer Ken" && helper ? true : `value=${v} helper=${helper}`;
  });
  await check("focus stays in the field after re-render", async () =>
    (await page.evaluate(() => document.activeElement.placeholder)) === "Your name");
  await check("error state toggles", async () => {
    await page.fill("input[type=email]", "nope");
    await app.settle();
    return page.isVisible("text=Must contain @");
  });
  await check("uncontrolled field keeps user text across re-renders", async () => {
    await page.fill("input[type=password]", "changed");
    await page.click('.skeuo-btn:has-text("secondary") >> nth=0');
    await app.settle();
    return (await page.inputValue("input[type=password]")) === "changed";
  });
  await check("appearance switch: Dark", async () => {
    await page.click('[role=tab]:has-text("Dark")');
    await app.settle();
    return (await page.getAttribute(".skeuo-root", "data-skeuo-mode")) === "dark";
  });
  if (shots) await page.screenshot({ path: `${shots}/live-dark.png` });
  await check("sidebar navigation", async () => {
    await page.click('.gallery__nav-item:has-text("Icons")');
    await app.settle();
    return (await page.textContent(".gallery__main h1")) === "Icons";
  });
  await check("state resets when a page is left and re-entered", async () => {
    await page.click('.gallery__nav-item:has-text("Inputs")');
    await app.settle();
    return (await page.textContent("text=/Clicked \\d+ times/")).includes("0");
  });

  console.log(results.join("\n"));
  console.log(`${app.exchanges} messages exchanged; ${app.errors.length ? "page errors: " + app.errors.join(" | ") : "no page errors"}`);
  await app.close();
  process.exit(results.some((r) => r.startsWith("FAIL")) ? 1 : 0);
})();
