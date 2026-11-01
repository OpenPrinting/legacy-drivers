# Contributing to legacy-drivers

Thank you for helping preserve historical Linux printer drivers.

The purpose of this repository is to preserve old and abandoned printer drivers that are still useful for historical printers but whose original upstream projects are no longer maintained.

Our main goal is **preservation**.

We want to keep the original upstream source available and reproducible while also providing the patches and build instructions needed to build and use the driver on modern systems.

A key principle is:

> **Keep the original source untouched. Put modernization and preservation changes into patches.**

This is especially important because we usually do not have the original printer hardware available for testing. Unnecessary changes to an old driver can therefore break its behavior without us being able to detect the regression.

---

## 1. Find a candidate driver

There are several ways to find old printer drivers.

One useful approach is to look through Linux distributions that still carry historical printer drivers.

For example, inspect the printer drivers included in:

- Debian
- Ubuntu
- Fedora
- openSUSE
- Mageia
- Other Linux distributions
- Old distribution releases

Distribution package repositories are useful because they can contain drivers that are no longer maintained by their original upstream projects.

For each old printer driver you find, investigate whether its **original upstream project is still active or has been abandoned**.

Other useful places to search include:

- Historical Linux software archives
- Old FTP mirrors
- Project websites
- Internet Archive / Wayback Machine
- Old software CDs
- Source archives
- Old mailing lists
- Historical distribution source packages

Do not immediately modify or modernize a driver when you find it.

First investigate its origin and history.

---

## 2. Investigate the candidate

Before adding a driver, establish as much information as possible about its original upstream project.

Try to find:

- Driver/project name
- Original author(s)
- Original project homepage
- Original source repository, if one existed
- Original download URL
- Original FTP location
- Original archive filename
- Original version
- Original release date
- Historical releases
- Last known upstream release
- License
- Historical documentation
- Wayback Machine history

A distribution package is useful evidence, but **a distribution package is not automatically the original upstream source**.

---

## 3. Establish source provenance

Determine where the source originally came from.

Whenever possible, compare copies of the driver from different sources.

Useful sources include:

- Original upstream archives
- Wayback Machine snapshots
- Debian source packages
- Ubuntu source packages
- Fedora source packages
- RPM packages
- Other historical distributions

Clearly distinguish between:

1. Original upstream source
2. Distribution-specific changes
3. OpenPrinting changes
4. Your own changes

Do not mix these together.

If a distribution still ships an old driver, this does **not** necessarily mean that the original upstream project is still active.

Distribution maintenance and upstream maintenance are different things.

---

## 4. Check whether upstream is abandoned

This is an important part of the investigation.

Look for evidence about the original upstream project's development history.

Useful evidence includes:

- Last upstream release
- Last source release
- Last project website update
- Last mailing-list activity
- Last upstream commit, if a repository existed
- Project closure or abandonment announcement
- Lack of newer upstream releases
- Historical development discussions

Do not decide that a project is abandoned only because the source is old.

Try to establish the status from multiple sources.

The important question is:

> **Is the original upstream project still actively maintained?**

---

## 5. Check the license

Inspect the **original upstream source archive** when possible.

Look for:

- `LICENSE`
- `COPYING`
- `README`
- Copyright notices
- Source-file headers
- Distribution statements
- Public-domain statements
- Author permissions

Record where the license information was found.

Do not assume that a driver is redistributable just because a Linux distribution packages it.

If the original license is unclear, discuss it with the maintainers before adding the source.

---

## 6. Preserve the original source

The original upstream source is the most important part of the preservation effort.

**Do not modify the original source directly.**

Keep the original source as close as possible to the historical upstream release.

Do not:

- Rewrite the driver
- Modernize the source directly
- Replace the original build system
- Reformat the whole source tree
- Remove old files because they appear unnecessary
- Change copyright notices
- Change license text
- Silently apply distribution patches
- Silently apply OpenPrinting patches

Historical files can be important for understanding and reproducing the original driver.

When in doubt, preserve the original file and discuss changes with the maintainers.

---

## 7. Use patches for modernization and preservation

Modernization should be provided through patches.

The original source must remain identifiable and recoverable.

A driver directory should conceptually look like:

```text
driver-name/
├── original-source/
│   └── driver-version/
│       ├── ...
│       └── original files
│
├── patches/
│   ├── debian/
│   ├── ubuntu/
│   ├── fedora/
│   └── openprinting/
│
└── README.md
```

The exact directory names may vary according to the repository's organization, but the principle should remain the same.

Patches originating from different distributions should be kept separate. Changes created by OpenPrinting should also be kept separately.

This makes it possible to determine where each change came from.

---

## 8. Keep patching to a minimum

Be very careful when modifying an old driver.

We often do not have the original printer that the driver was designed for. Therefore, we may not be able to test whether a modification changes the printer's behavior.

The fewer changes we make to the original driver behavior, the lower the risk of breaking the driver.

> **Do not change something just because it looks old. Change it only when there is a clear reason.**

For example, a compiler compatibility problem may require a small patch. Do not rewrite unrelated parts of the driver simply because they use old programming techniques.

---

## 9. Preserve distribution patches separately

A distribution may already have patches that make the driver build or work on newer systems.

Do not automatically copy all distribution changes into the original source.

Instead, identify and preserve them as patches.

For example:

```text
original-source/
    driver-version/

patches/
    debian/
        0001-build-fix.patch

    ubuntu/
        0001-compiler-fix.patch

    openprinting/
        0001-modern-build-fix.patch
```

This allows us to see which changes came from which project and keeps the original source recoverable.

---

## 10. Document how to build the driver

Every driver directory should contain clear instructions for building and installing that driver.

At minimum, document:

- Required operating system/environment
- Required compiler
- Required libraries
- Required development packages
- Required tools
- Environment variables
- Patch order
- Which patches are optional
- Which patches are required
- `./configure` options
- `make` options
- Installation commands
- Any special configuration
- Known build problems

For example:

```text
1. Extract the original source.
2. Apply the required distribution patches.
3. Apply the OpenPrinting patches.
4. Set the required environment variables.
5. Run the documented ./configure command.
6. Run the documented make command.
7. Install using the documented instructions.
```

The actual commands must be based on the driver being preserved.

Do not invent build instructions without testing them.

---

## 11. Document environment variables

If the driver requires environment variables, document them clearly.

For example:

```bash
export CFLAGS="-O2"
export CPPFLAGS="-D..."
```

Explain why they are required.

If an environment variable is only required for a specific build environment, say so.

---

## 12. Document configure and make options

If the driver uses Autotools or another configure-based build system, document the exact options needed.

For example:

```bash
./configure \
    --prefix=/usr \
    --sysconfdir=/etc
```

If special `make` options are required, document them too:

```bash
make -j2
```

Do not replace the historical build system unnecessarily.

If a modern build system is required, provide that modernization as a patch.

---

## 13. Test the original source first

Before applying modernization patches, try to build the original upstream source if practical.

Document:

- Operating system
- Architecture
- Compiler version
- Required dependencies
- Build command
- Build result
- Errors
- Warnings
- Missing dependencies

A build failure does not automatically mean that the driver should be rejected.

The purpose of this step is to understand and document the original state.

---

## 14. Test patched builds separately

After documenting the original build, test the patched version.

Document the result for each important patch set.

For example:

```text
Original source:
  Does not build with GCC 14.

Distribution patches:
  Build succeeds.

OpenPrinting patches:
  Build succeeds with modern environment.

Final build command:
  ...
```

Because the original printer may not be available, avoid claiming that the driver is fully tested unless it actually has been tested with the hardware.

---

## 15. Do not claim hardware compatibility without testing

A successful compilation does not prove that a printer driver works.

```text
Build succeeds
    !=
Printer works
```

If the original printer is unavailable, say so.

For example:

```text
The driver builds successfully with GCC 14.

The original printer hardware was not available for testing.
```

This is especially important for legacy drivers.

---

## 16. Check for existing copies

Before adding a driver, search this repository for related work.

Check:

- Driver name
- Version
- Archive name
- Author
- Source files
- Printer models
- Historical project URLs

Avoid adding duplicate copies of the same upstream project.

If a related driver already exists, explain why the proposed contribution is different.

---

## 17. Verify the source archive

Record useful information about the preserved source.

For example:

```text
Filename:
    driver-version.tar.gz

Version:
    x.y.z

Release date:
    YYYY-MM-DD

SHA256:
    <hash>
```

If several historical copies exist, compare them.

If two archives have the same checksum, they contain the same bytes.

If they differ, investigate why and document the reason when known.

---

## 18. Prepare a provenance record

Before opening a pull request, collect the investigation results.

A driver should have enough documentation for another person to understand where the source came from.

For example:

```text
Driver:
Version:
Original archive:
Release date:

Original author:
Original project:
Original URL:
Original download URL:

Historical mirrors:
Wayback references:

License:
License location:

Last known upstream release:
Evidence of abandonment:

Distribution copies:
Debian:
Ubuntu:
Fedora:
Other:

Original build status:
Patched build status:

Build environment:
Required environment variables:
Configure options:
Make options:
Patch order:

SHA256:

Hardware testing:
```

---

## 19. Open an issue for uncertain candidates

If the provenance, license, upstream status, or preservation method is unclear, open an issue before adding the driver.

Explain:

- What you found
- Where you found it
- Why it appears to be a historical printer driver
- Who appears to be the original author
- What the original upstream project was
- What evidence shows that upstream is abandoned
- What license applies
- What source versions you found
- What is still uncertain

Ask the maintainers whether the driver is suitable for the repository.

---

## 20. Prepare the driver directory

Each preserved driver should contain:

1. The original upstream source
2. Patches
3. Build/install instructions
4. Provenance information
5. License information
6. Information about patch origin
7. Information about the build environment

A typical structure is:

```text
driver-name/
├── original-source/
│   └── driver-version/
│
├── patches/
│   ├── debian/
│   ├── ubuntu/
│   ├── fedora/
│   └── openprinting/
│
└── README.md
```

The `README.md` should clearly explain how to reproduce the final build from the preserved original source.

---

## 21. Create the pull request

Once the investigation is complete:

1. Preserve the original upstream source.
2. Add provenance information.
3. Add the original version and release information.
4. Document the license.
5. Add checksums where appropriate.
6. Keep distribution patches separate.
7. Keep OpenPrinting patches separate.
8. Document patch order.
9. Document environment variables.
10. Document `configure` options.
11. Document `make` options.
12. Document installation steps.
13. Document build results.
14. Clearly state whether the original hardware was available for testing.
15. Explain why the driver belongs in `legacy-drivers`.

The pull request should allow a maintainer to understand:

> **What is this driver? Where did it come from? Is the upstream project abandoned? What source are we preserving? What patches are being applied? How can someone reproduce the build?**

---

# Example investigation

Suppose you find an old archive:

```text
cjet089.tgz
dated: 1996-03-15
```

Do not immediately add it.

Follow the investigation:

```text
1. Find the driver in a historical source
                |
                v
2. Identify the original project
                |
                v
3. Identify the original author
                |
                v
4. Find the original release
                |
                v
5. Find the original download URL
                |
                v
6. Check Wayback and historical mirrors
                |
                v
7. Check the original license
                |
                v
8. Determine the last upstream release
                |
                v
9. Determine whether upstream is abandoned
                |
                v
10. Compare Debian/Ubuntu/other copies
                |
                v
11. Separate upstream source from
    distribution patches
                |
                v
12. Preserve the original source
                |
                v
13. Test the original build
                |
                v
14. Create minimal modernization patches
                |
                v
15. Test the patched build
                |
                v
16. Document patch order and build commands
                |
                v
17. Open an issue or pull request
```

---

# What not to do

## Do not modernize the original source directly

Avoid:

```text
Old source
   |
   v
Rewrite source
   |
   v
Commit rewritten source
```

Prefer:

```text
Original source
   |
   +---- patch 1
   |
   +---- patch 2
   |
   +---- OpenPrinting patch
   |
   v
Buildable driver
```

## Do not assume a distribution package is upstream

A distribution source package can contain:

```text
Upstream source
+
Distribution patches
+
Distribution build files
+
Distribution-specific changes
```

Identify which parts came from upstream before preserving them.

## Do not apply every available patch automatically

Investigate each patch:

- Why was it created?
- Who created it?
- What problem does it solve?
- Does it change driver behavior?
- Is it still required?
- Is it safe to apply?
- Can it be reproduced?

Keep only the changes that are appropriate and documented.

## Do not make large changes when a small patch is enough

If one compiler error can be fixed with a small patch, do not rewrite the entire source file.

Prefer:

```text
One required change
        |
        v
Small patch
        |
        v
Original behavior preserved
```

Remember:

> **We often cannot test the original printer.**

Therefore, unnecessary changes increase the risk of breaking something we cannot verify.

## Do not remove historical files casually

Old files may contain documentation, build information, copyright information, examples, printer-specific information, or other historical clues.

If a file appears unnecessary, preserve it unless there is a clear reason to remove it.

Discuss questionable removals with maintainers.

---

# Contributor checklist

Before opening a pull request, make sure you can answer these questions.

## Candidate and provenance

- [ ] Did I search Linux distributions such as Debian and Ubuntu for old printer drivers?
- [ ] Did I identify the original upstream project?
- [ ] Did I identify the original author, if possible?
- [ ] Did I identify the original version?
- [ ] Did I identify the original archive?
- [ ] Did I identify the original release date?
- [ ] Did I find the original project URL?
- [ ] Did I find the original download URL?
- [ ] Did I check historical mirrors?
- [ ] Did I check Wayback history?
- [ ] Did I establish the source provenance?
- [ ] Did I investigate the last upstream release?
- [ ] Did I establish whether upstream is abandoned?

## License

- [ ] Did I inspect the original upstream license?
- [ ] Did I record where the license was found?
- [ ] Did I verify that preservation and redistribution are permitted?
- [ ] Did I discuss unclear licensing with maintainers?

## Source preservation

- [ ] Did I preserve the original upstream source?
- [ ] Did I avoid modifying the original source directly?
- [ ] Did I avoid unnecessary removal of historical files?
- [ ] Did I record the original archive checksum?

## Patches

- [ ] Did I distinguish upstream source from distribution changes?
- [ ] Are Debian patches kept separately?
- [ ] Are Ubuntu patches kept separately?
- [ ] Are other distribution patches kept separately?
- [ ] Are OpenPrinting patches kept separately?
- [ ] Is the patch order documented?
- [ ] Are patches kept as small as reasonably possible?
- [ ] Did I avoid unnecessary changes to driver behavior?

## Build instructions

- [ ] Did I test the original source where practical?
- [ ] Did I document the build environment?
- [ ] Did I document required dependencies?
- [ ] Did I document required environment variables?
- [ ] Did I document the required `configure` options?
- [ ] Did I document the required `make` options?
- [ ] Did I document the patch order?
- [ ] Did I document installation instructions?
- [ ] Did I test the patched build?
- [ ] Did I document build failures and warnings?

## Hardware testing

- [ ] Did I clearly state whether the original printer hardware was available?
- [ ] Did I avoid claiming hardware compatibility without testing?
- [ ] Did I make the minimum changes necessary because the original hardware may not be available?

## Repository

- [ ] Did I search for an existing copy of the driver?
- [ ] Did I avoid creating a duplicate?
- [ ] Does the driver directory contain the original source?
- [ ] Does the driver directory contain the required patches?
- [ ] Does the driver directory contain clear build/install instructions?
- [ ] Is the provenance information documented?
- [ ] Is the license information documented?

---

# Guiding principles

The `legacy-drivers` project follows a few simple principles.

### 1. Preserve the original

The historical upstream source is valuable.

Keep it intact and identifiable.

### 2. Patch instead of rewriting

Modernization and preservation changes should be provided as patches.

This keeps the original source recoverable.

### 3. Keep patches small

Every unnecessary change increases the possibility of changing the driver's behavior.

This is especially important when the original printer is no longer available for testing.

### 4. Keep patch origins clear

Distribution patches and OpenPrinting patches should remain distinguishable.

This makes the history of the driver easier to understand.

### 5. Make builds reproducible

A contributor should be able to follow the documented instructions and understand how the preserved source becomes a buildable driver.

### 6. Document uncertainty

If something is unknown, say so.

Do not guess the original author, license, URL, release date, or hardware compatibility.

### 7. Preservation comes first

The goal is not to make an old driver look like a modern software project.

The goal is to preserve the driver and provide the minimum changes and clear instructions needed to build and use it today.

---

# Final principle

When working on a legacy printer driver, think like an archivist first and a developer second.

```text
Find it
   |
   v
Identify its origin
   |
   v
Verify its history
   |
   v
Verify its license
   |
   v
Determine upstream status
   |
   v
Preserve the original source
   |
   v
Separate distribution patches
   |
   v
Add minimal OpenPrinting patches
   |
   v
Document the build environment
   |
   v
Document patch and build order
   |
   v
Test what can be tested
   |
   v
Submit the contribution
```

> **Preserve first. Patch carefully. Change as little as possible. Document everything.**
