// Drives a skeuo-ui C++ app in headless Chromium for interaction tests.
//
//   const { launch } = require('./harness');
//   const app = await launch('build/gallery');   // runs `gallery --stdio`
//   await app.page.click('text=Save');          // events go to the C++ process
//   await app.close();
//
// The app's stdio host prints {"t":"page","html":...} first, then answers
// every JSON line it is sent. The page's runtime talks to it through
// window.__skHostSend, which this harness provides.
//
// Needs playwright-core; pass its path with PLAYWRIGHT_CORE or install it next to
// this file. Chromium is taken from CHROMIUM (default /opt/pw-browsers/chromium).
const { spawn } = require("child_process");
const readline = require("readline");

function loadPlaywright() {
  const path = process.env.PLAYWRIGHT_CORE || "playwright-core";
  return require(path);
}

async function launch(binary, { args = [], viewport = { width: 1280, height: 900 }, colorScheme = "light" } = {}) {
  const { chromium } = loadPlaywright();
  const child = spawn(binary, ["--stdio", ...args], { stdio: ["pipe", "pipe", "inherit"] });
  const lines = readline.createInterface({ input: child.stdout });
  const waiting = [];
  const pending = [];
  lines.on("line", (line) => {
    const resolve = waiting.shift();
    if (resolve) resolve(line);
    else pending.push(line);
  });
  const nextLine = () => new Promise((resolve) => {
    if (pending.length) resolve(pending.shift());
    else waiting.push(resolve);
  });

  const first = JSON.parse(await nextLine());
  const browser = await chromium.launch({ executablePath: process.env.CHROMIUM || "/opt/pw-browsers/chromium" });
  const page = await browser.newPage({ viewport, colorScheme });
  const errors = [];
  page.on("pageerror", (e) => errors.push(e.message));
  page.on("console", (m) => { if (m.type() === "error") errors.push(m.text()); });
  let exchanges = 0;
  await page.exposeFunction("__skHostSend", async (msg) => {
    exchanges++;
    child.stdin.write(msg + "\n");
    return nextLine();
  });
  await page.setContent(first.html, { waitUntil: "load" });

  return {
    page,
    errors,
    get exchanges() { return exchanges; },
    /// Waits until queued events have been answered and patched in.
    async settle(ms = 150) { await page.waitForTimeout(ms); },
    async close() {
      await browser.close();
      child.stdin.end();
      child.kill();
    },
  };
}

module.exports = { launch };
