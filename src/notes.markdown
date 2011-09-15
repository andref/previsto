Developer Notes
===============

Features
--------

These are missing:

- Switch stylesheets (light, dark, sans, serif)
- An about box...
- Print the document
- Export to PDF (**with pagination**, unlike Marked!)

Error Checking
--------------

- **The multimarkdown binary is not available.** The installation must be broken: we store the `multimarkdown` binary at `/usr/local/lib/previsto`. Can we try and reconfigure the package?

- **The document does not exist.** There is nothing we can do about it.

- **The document is not readable.** We can try and change the permissions automatically. If that fails, there is nothing we can do about it.

- **The multimarkdown program crashes.** There is nothing we can do about it.

- **The multimarkdown program takes too long to finish.** We can offer the user to kill it or wait a bit more. Must remember to close the notification if the program finishes before the user takes action.

Packaging
---------

Ubuntu complained that our package is low-quality---how dare they! Must find a tool that is not as complicated as the `dh_make` stuff. Currently, we have the following issues:

- The `md5sums` file is not being generated.
- Information about versioning, etc, is being kept on more than one place.

