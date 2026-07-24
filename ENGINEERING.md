# Drogular Engineering Principles

This document defines the engineering principles that guide the development of Drogular.

The goal of these principles is to keep the framework simple, consistent, production-ready, and maintainable for many years.

---

# 1. Production before convenience

A feature must prove its value in a real application before becoming part of the framework.

Every public API should be validated in production-oriented Demo applications before it is added to Drogular Core.

---

# 2. Extract, don't invent

Core APIs are extracted from real applications.

We do not design abstractions in advance.

We first solve a real problem, identify recurring patterns, and only then move the abstraction into the framework.

---

# 3. Simplicity over cleverness

Readable code is preferred over clever code.

Simple implementations are easier to understand, maintain, optimize and extend.

Whenever two solutions solve the same problem, the simpler one should be preferred.

---

# 4. Consistency over flexibility

Drogular intentionally favors one recommended approach over multiple equivalent alternatives.

Consistency reduces cognitive load and makes projects easier to read and maintain.

---

# 5. Architecture before implementation

Significant features begin with architectural design.

Implementation follows architecture—not the other way around.

Good architecture reduces complexity before a single line of code is written.

---

# 6. Documentation is architecture

Documentation is not an afterthought.

It explains both:

- how Drogular works
- why Drogular works this way

Every architectural decision should eventually be reflected in the documentation.

---

# 7. Examples are production

Examples are taken from real Demo applications.

Documentation should never depend on isolated example projects that diverge from production code.

Demo applications are the primary source of examples.

---

# 8. Every abstraction must earn its place

Every abstraction added to Drogular Core should answer three questions:

1. Which real problem does it solve?
2. Which duplication does it eliminate?
3. Why does it belong in the framework instead of the application?

If these questions cannot be answered convincingly, the abstraction should remain in the application.

---

# 9. Stable APIs matter

Public APIs are long-term commitments.

Breaking changes should be rare and carefully justified.

Backward compatibility is considered part of the framework's quality.

---

# 10. Build for the next decade

Engineering decisions should support long-term evolution.

This does not mean premature optimization.

It means avoiding short-term solutions that will require redesign after a few releases.

---

# 11. Keep the Core small

Every line added to the framework becomes a long-term maintenance responsibility.

New functionality should be added only when it provides substantial value to a broad range of applications.

Whenever possible, features should remain in libraries, extensions, or applications instead of the Core.

---

# Feature Lifecycle

Every feature follows the same development process.

```text
Idea
    ↓
Architecture
    ↓
Implementation
    ↓
Tests
    ↓
Demo Application
    ↓
Documentation
    ↓
Release
```

A feature is considered complete only after the final step.

---

# Documentation Lifecycle

Documentation follows implementation.

A feature is documented only after it has been:

- implemented;
- tested;
- validated in a Demo application.

Documentation reflects proven design rather than planned functionality.

---

# Project Philosophy

Drogular aims to be:

- simple instead of clever;
- consistent instead of configurable;
- production-oriented instead of feature-oriented;
- stable instead of experimental.

The framework grows by extracting successful patterns from real applications rather than by predicting future requirements.
