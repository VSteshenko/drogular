#pragma once

#include <drogular/page.hpp>
#include <drogular/render_context.hpp>

class JobsDemoHomePage final : public drogular::Page {
public:
    std::string render(drogular::RenderContext&) override {
        return R"HTML(<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>JobsDemo · Drogular</title>
    <link rel="stylesheet" href="/assets/jobs.css">
    <script type="module" src="/assets/jobs.js"></script>
</head>
<body>
    <header class="top">
        <div>
            <strong>Drogular</strong>
            <span class="separator">/</span>
            JobsDemo
        </div>
        <a href="/__drogular">Developer Tools</a>
    </header>
    <main>
    <div class="hero">
        <div>
            <p class="eyebrow">Drogular 0.26 · Background Jobs</p>
            <h1>Job Runtime</h1>
            <p>Launch work, follow progress via SSE, and inspect runtime diagnostics.</p>
        </div>
        <div class="hero-actions">
            <label for="duration">Duration</label>
            <select id="duration">
                <option value="5">5 seconds</option>
                <option value="15" selected>15 seconds</option>
                <option value="30">30 seconds</option>
                <option value="60">60 seconds</option>
                <option value="120">120 seconds</option>
            </select>
            <button id="launch">Run job</button>
            <button id="fail" class="secondary">Run failing job</button>
        </div>
    </div>
    <nav aria-label="Views">
        <button data-tab="jobs" class="selected" aria-current="page">Jobs</button>
        <button data-tab="diagnostics">Diagnostics</button></nav>
        <div id="notice" role="status" aria-live="polite"></div>
        <section id="jobs-panel">
            <div class="heading">
                <h2>Recent jobs</h2>
                <span id="connection" class="status">Connecting…</span>
            </div>
            <div id="jobs" class="job-list">
                <p class="muted">No jobs yet. Run a job to see live updates.</p>
            </div>
        </section>
        <section id="diagnostics-panel" hidden>
            <div class="heading">
                <h2>Runtime diagnostics</h2>
                <span class="muted">Live counters</span>
            </div>
            <div id="metrics" class="metrics"></div>
            <p class="muted">Complete Drogular inspection: <a href="/__drogular">Developer Tools</a></p>
        </section>
    </main>
</body>
</html>)HTML";
    }
};