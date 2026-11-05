<p align="center">
    <img src="assets/logo.png" width="200" alt="Drogular-Logo">
</p>

# Drogular

**Sprache:** [English](README.md) | **Deutsch**

> **Modernes C++-Anwendungsframework auf Basis von Drogon mit servergesteuerter Benutzeroberfläche, GraphQL, PWA und erweiterbaren Entwicklerwerkzeugen.**

Mit Drogular lassen sich moderne Webanwendungen in C++ entwickeln – auf Grundlage einer einheitlichen Architektur für Komponenten, Dependency Injection, Zustandsverwaltung, GraphQL und Entwicklerwerkzeuge.

**Ein zusammenhängendes Anwendungsframework – keine Sammlung voneinander unabhängiger Bibliotheken.**

---

## Warum Drogular?

Für moderne Webanwendungen in C++ müssen häufig separate Bibliotheken für Routing, Templates, Dependency Injection, Zustandsverwaltung, GraphQL-Clients und Entwicklungswerkzeuge kombiniert werden.

Drogular verfolgt einen anderen Ansatz.

Diese Funktionen sind Bestandteile eines einheitlichen Anwendungsframeworks mit einem konsistenten Programmiermodell. Seiten, Komponenten, Dienste, Aktionen, Zustandsverwaltung, GraphQL-Integration und Entwicklerwerkzeuge sind darauf ausgelegt, zusammenzuarbeiten, statt aus unabhängigen Bibliotheken zusammengesetzt zu werden.

---

## Grundprinzipien

- 🧩 Komponentenbasierte Architektur
- ⚡ Serverseitig gerenderte Benutzeroberfläche mit gezielter progressiver Erweiterung
- 💉 Durchgängige Dependency Injection
- 🔄 Reaktive Zustandsverwaltung
- 🌐 Native GraphQL-Integration
- 🌍 Integrierte Lokalisierung
- 📱 Progressive Web Apps
- 🛠 Entwicklerwerkzeuge auf Basis öffentlicher APIs
- 🔌 Erweiterbare Architektur
- 🚀 Integrierter Projektgenerator und Entwickler-CLI

---

## Architektur

```text
Application
    ├── Pages
    ├── Components
    ├── Services
    ├── Actions
    ├── State
    ├── GraphQL
    ├── Localization
    └── Developer Tools
```

Alle Teilsysteme folgen denselben Architekturprinzipien und ermöglichen so ein konsistentes Programmiermodell innerhalb der gesamten Anwendung.

---

## Entwicklerwerkzeuge

Die Entwicklerwerkzeuge sind ein fester Bestandteil von Drogular und kein externes Hilfsprogramm.

```text
             Application

                   │
                   ▼
       Application Inspection

                   │
                   ▼
          Inspection JSON API

        ┌──────────┴──────────┐
        ▼                     ▼
 Diagnostics Page      External Tools
```

Die Entwicklerwerkzeuge umfassen:

- Entwicklungsprofil
- Integrierte Diagnosefunktionen mit aggregierten Laufzeitmetriken
- Anwendungsinspektion
- Erweiterbare Inspektionsbeiträge
- Benutzerdefinierte Entwicklerkomponenten
- Öffentliche JSON-Schnittstelle für externe Werkzeuge
- Für IDE-Integration ausgelegte Architektur

Anwendungen und Bibliotheken können eigene Inspektionsbereiche und Visualisierungen beisteuern, ohne Drogular selbst verändern zu müssen.

---

## Referenzanwendung

**PortalDemo** ist die Referenzanwendung von Drogular.

Sie demonstriert die empfohlene Architektur, einschließlich:

- Authentifizierung
- Dependency Injection
- Komponenten
- Reaktiver Zustandsverwaltung
- GraphQL
- Lokalisierung
- Entwicklerwerkzeugen

PortalDemo ist keine Sammlung isolierter Beispiele, sondern zeigt, wie diese Funktionen in einer vollständigen Anwendung zusammenspielen.

---

## Beispiele

| Beispiel | Demonstrierte Funktionen |
|----------|--------------------------|
| TodoPWA | Komponenten, Zustandsverwaltung, Formulare und Validierung |
| Auth Sample | Authentifizierung und Sitzungen |
| Developer Tools | Erweiterung der Entwicklerwerkzeug-Plattform |
| Repository Sample | Repository-Muster und Datenzugriff |
| System Monitor PWA | Serverseitig gerenderte Live-Fragmente, PWA-Verhalten und Fernüberwachung von System und Hardware |
| PortalDemo | Vollständige Architektur einer Referenzanwendung |
| Mini Store | Lokalisierter Storefront- und Staff-Workflow, Warenkorb/Checkout, Bestelllebenszyklus, Interactions und PWA-Verhalten |

---

## Schnellstart

Am schnellsten erstellen Sie eine Drogular-Anwendung mit der CLI:

```bash
drogular new hello_drogular
cd hello_drogular
cmake -S . -B build
cmake --build build
./build/hello_drogular
```

Das generierte Projekt ist auf die passende Drogular-Version abgestimmt und enthält eine Page, eine wiederverwendbare Component, Templates, öffentliche Assets und die empfohlene Startstruktur.

Die CLI unterstützt außerdem explizite Ausgabepfade und mehrere integrierte Projektvorlagen:

```bash
drogular templates
drogular new examples/admin --template minimal
drogular new apps/MyPWA --template pwa
```

`minimal` ist die kleinste empfohlene Drogular-Anwendung. `pwa` ist eine direkt ausführbare, installierbare Projektvorlage mit Manifest, Service Worker, Offline-Fallback, responsiver Benutzeroberfläche und kurzen `Tip:`-Kommentaren zu wichtigen Erweiterungspunkten.

Für eine minimale Route genügen eine Page und `drogular::App`:

```cpp
#include <drogular/app.hpp>
#include <drogular/page.hpp>
#include <drogular/render_context.hpp>

#include <string>

class HomePage final : public drogular::Page
{
public:
    std::string render(drogular::RenderContext&) override
    {
        return "<h1>Hello Drogular</h1>";
    }
};

int main()
{
    drogular::App app;

    app.page<HomePage>("/");
    app.run(8080);

    return 0;
}
```

Die Anleitung [Getting Started](docs/getting-started/README.md) erläutert die erforderliche CMake-Konfiguration, Templates, Components, Dienste, Dependency Injection, den CLI-Workflow und die Projektorganisation.

---

## Funktionsübersicht

| Funktion | Status |
|----------|:------:|
| Komponenten | ✅ |
| Routing | ✅ |
| Dependency Injection | ✅ |
| Zustandsverwaltung | ✅ |
| Formulare und Validierung | ✅ |
| GraphQL-Integration | ✅ |
| Lokalisierung | ✅ |
| Progressive Web Apps | ✅ |
| Entwicklerwerkzeuge | ✅ |

---

## Neu in Version 0.25

Drogular 0.25 ergänzt Client-/Offline-Lokalisierung, sichere Rücksprungnavigation und interaktive Aufgaben in PortalDemo.

- 🌍 `ClientLocalizationCatalog` erzeugt deterministische Manifest- und Sprachressourcen aus dem serverseitigen Übersetzungskatalog.
- 📱 `App::clientLocalization()` stellt die Browser-Laufzeit mit `configure()`, `ready()`, `setLocale()` und `translate()` bereit. Die Fallback-Reihenfolge bleibt angeforderte Sprache → Standardsprache → Schlüssel.
- 📴 Versionierte Sprachressourcen werden unabhängig vom Service-Worker-Shell-Cache in einem eigenen Cache-Storage-Namensraum gespeichert.
- ↩️ `NavigationContext` prüft und überträgt sichere lokale Rücksprungziele.
- 📋 PortalDemo verwendet eigenständige Aufgaben mit Status, Priorität, einem gemeinsamen Demo-Datensatz und autorisierten interaktiven Änderungen.
- 🧪 Regressionstests decken Lokalisierung, Navigation, Aufgabenänderungen und die Service-Worker-Cache-Isolation ab.

Details: [RELEASE_NOTES_0.25.md](RELEASE_NOTES_0.25.md).

---

## Dokumentation

- 📖 [Getting Started](docs/getting-started/README.md)
- 🍳 [Cookbook](docs/cookbook/README.md)
- 🏗 [Architecture](docs/architecture/README.md)
- 📚 [API Reference](docs/reference/README.md)
- 🧩 [Examples](examples/)
- 🚀 [PortalDemo – Referenzanwendung](examples/portal_demo/)
- 🛍 [Mini Store – Referenzanwendung](examples/mini_store/)

---

## Roadmap

### 0.26 — Hintergrundaufgaben und Fortschritt

Ein einheitliches Modell für lang laufende und im Hintergrund ausgeführte Aufgaben einführen. So können Anwendungen wie Atlasys Vorgänge starten, beobachten und steuern, ohne deren Lebensdauer an eine einzelne HTTP-Anfrage zu binden.

- Ein Framework-weites Job-Modell mit Identität, Lebenszyklusstatus, Ergebnis-/Fehlerstatus und Ausführungsmetadaten hinzufügen
- Fortschrittsmeldungen für lang laufende Jobs bereitstellen
- Kooperative Job-Abbrüche mit eindeutigem Abbruchstatus und klar definiertem Verhalten ermöglichen
- Einen SSE-/Event-Stream für Live-Aktualisierungen von Job-Status und Fortschritt hinzufügen
- `JobsDemo` als Referenzanwendung zum Starten, Beobachten, Abbrechen und erneuten Verbinden mit Jobs ergänzen
- Job-Diagnose für Zustandsübergänge, Ausführungsdauer, Fehler, Abbrüche sowie die Inspektion aktiver und abgeschlossener Jobs hinzufügen

### 0.27 — Health, Readiness und Authentifizierungskontext

Betriebliche Grundlagen und Identitätskontext bereitstellen, die Anwendungen wie Atlasys für den Produktivbetrieb benötigen.

- Standardisierte Health- und Readiness-Endpunkte mit erweiterbaren anwendungsspezifischen Prüfungen hinzufügen
- Das Authentifizierungsmodell um einen umfangreicheren Principal-/Authentifizierungskontext für die Autorisierung von Anwendungen und Diensten erweitern
- `AuthSample` erweitern, um Principal-Daten, Authentifizierungsstatus, autorisierungsbezogene Nutzung und neue Auth-Erweiterungspunkte zu demonstrieren

### 0.28 — Audit und Metriken

Framework-weite Erweiterungspunkte für Observability einführen, damit Anwendungen Drogular-Aktivitäten mit Betriebs-, Compliance- und Monitoring-Infrastrukturen verbinden können.

- Audit-Hooks für sicherheitsrelevante und anwendungsspezifische Vorgänge hinzufügen, ohne das Framework an ein bestimmtes Audit-Backend zu koppeln
- Eine Metrics-Exporter-Abstraktion bereitstellen, um Framework- und Anwendungsmetriken an externe Monitoringsysteme zu übergeben

### 0.29 — Windows-Unterstützung und plattformübergreifende Stabilisierung

Die unterstützten Plattformen erweitern, ohne den geplanten Umfang von 0.25–0.28 zu verändern.

- Windows-x64-Builds mit MSVC und Visual Studio unterstützen
- Windows-CI mit vollständiger Testsuite hinzufügen
- Die Portabilität zwischen Linux, macOS und Windows prüfen und verbessern
- MinGW zunächst außerhalb des Umfangs von 0.29 belassen

### 1.0

- Stabile öffentliche API
- Produktionsreife
- Langfristige API-Kompatibilität

---

## Designphilosophie

Drogular basiert auf einer einfachen Idee:

> **Jedes Teilsystem soll sich wie ein natürlicher Bestandteil des Frameworks anfühlen.**

Komponenten, Dependency Injection, Zustandsverwaltung, GraphQL-Integration, Lokalisierung und Entwicklerwerkzeuge folgen denselben Architekturprinzipien und schaffen ein konsistentes Programmiermodell für die gesamte Anwendung.

Das Ziel von Drogular besteht nicht nur darin, mehr Funktionen bereitzustellen, sondern ein zusammenhängendes Framework zu schaffen, in dem diese Funktionen selbstverständlich zusammenspielen.
