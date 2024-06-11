# Flatpak Platform for Gearsystem

[Flatpak](https://flatpak.org/) is a method of packaging and distributing
applications independent of the operating system and surrounding dependencies.
These applications also take advantage of sandboxing for security purposes, only
allowing applications access to services and files that they require.

Gearsystem can be built as a Flatpak application to take advantage of the
packaging format as well as the sandbox features available.

## Features

Building and distributing Gearsystem as a Flatpak has the following key
advantages:

* **Embedded SDL, GLU and GLEW libraries**. Compared to manual installation,
  all dependencies are included and sandboxed within the Gearsystem application.
  Outside of Flatpak's setup, no prerequisite commands are required!
* **Automatically created Desktop entries**. The Flatpak installation comes with
  the appropriate files to define desktop and menu entries for Gearsystem,
  allowing for the application to be quickly started from anywhere. These are
  also removed entirely when the application is uninstalled.
* **Consistent development environment**. Similar to how the application is
  sandboxed, the build environment is also sandboxed, ensuring all developers
  have the same experience when enhancing or troubleshooting Gearsystem.

This unfortunately comes with some trade-offs in functionality:

* **Adjacent Save files and Savestate files cannot be loaded**. The API used to
  load files into Gearsystem only loads specific files or directories. When
  this is done, only those file are permitted within the sandbox, resulting in
  adjacent files being invisible to the Gearsystem application. Instead, the
  application will only save files separate directory.
* **Some file paths may not reflect real paths**. As part of the above
  sandboxing, files can appear in Gearsystem to come from another location.
  These files are physically the same and use the same file name, but it can be
  a jarring first experience.

## Getting Started

This section is dedicated to preparing the development environment for
Gearsystem. This allows for building in-development versions and contributing
changes to Gearsystem. These instructions will predominantly use the
command-line and assume that `bash` is the shell being used.

1. **Install Flatpak and Flatpak builder**

   Flatpak can be installed by following [the quick setup steps for your
   distribution][Flatpak Setup]. Once Flatpak is installed, the
   `flatpak-builder` application must also be installed. This can be done
   either in a similar manner to how Flatpak was installed, or with `flatpak
   install flathub org.flatpak.Builder`. Once this is complete, Flatpak
   applications are built using one of the following:

   * `flatpak-builder`
   * `flatpak run org.flatpak.Builder`

   For the purposes of this tutorial, we will use `flatpak-builder`, but know
   that this can be interchanged with any of the above commands.

2. **Install the Runtime and SDK**

   Each Flatpak application has a base Runtime and SDK that they are built from,
   acting as a starting point for packages. These must be installed for builds
   to begin.

   For Gearsystem, the Runtime and SDK are `org.freedesktop.Platform` and
   `org.freedesktop.Sdk` respectively, running on version `23.08`. These can be
   installed via the command line with the following commands:

   ```bash
   flatpak install flathub \
       org.freedesktop.Platform//23.08 \
       org.freedesktop.Sdk//23.08
   ```

3. **Build the application**

   Builds are completed using `flatpak-builder` as follows (assuming
   `platforms/flatpak` is the current working directory):

   ```bash
   flatpak-builder \
       --force-clean \
       --user \
       --install \
       build-dir io.github.drhelius.Gearsystem.yml
   ```

   The `--force-clean --user --install` flags specify that the build should
   start from the beginning and, once completed, automatically install the
   application as a user-specific application. In these cases the application
   will be installed as `io.github.drhelius.Gearsystem//master`.

   The `build-dir` option indicates that any build files will be kept in a
   subdirectory with the same name. This may be deleted after a build is
   complete.

   The final option, `io.github.drhelius.Gearsystem.yml` indicates which
   manifest file to use for the application build. This manifest file contains
   the core configuration for how the application is built and any metadata
   associated with it. Some common modifications include changing the `make`
   command to `make DEBUG=1` to include debugging symbols or replacing the `git`
   source with a relative path for local development such as:

   ```yaml
   - type: dir
     path: ../..
   ```

   More information on this command and the manifest can be found on [the
   Flatpak Builder Command Reference][Flatpak Builder Reference].

4. **Run the application**

   Once installed, the application can be run with the following command:

   ```bash
   flatpak run io.github.drhelius.Gearsystem
   ```

   You may also be able to find the application in as a desktop entry, allowing
   you to search for it in your operating system's desktop environment.

   Starting this application in either manner should result in Gearsystem
   launching with full functionality.

   There are other options available to this command in [the Flatpak Command
   Reference][Flatpak Reference] if further debugging is available.

   One such option is the ability to debug the application with gdb. This can
   be done with the following commands:

   ```bash
   flatpak install gearsystem-origin io.github.drhelius.Gearsystem.Debug
   flatpak run \
       --devel \
       --command=gdb \
       io.github.drhelius.Gearsystem /app/bin/gearsystem
   ```

   The first command installs the `.Debug` extension with the debugging symbols
   from the local environment `gearsystem-origin`. This should be defined as you
   build the application, but if this does not work, simple run `flatpak list`
   and use the value in the `Origin` column corresponding to the `Gearsystem`
   application.

   The second command will start the GDB application on the Gearsystem binary.
   You may also run other applications such as `sh` for further debugging.

[Flatpak Builder Reference]: <https://docs.flatpak.org/en/latest/flatpak-builder-command-reference.html>
[Flatpak Reference]: <https://docs.flatpak.org/en/latest/flatpak-command-reference.html>
[Flatpak Setup]: <https://flatpak.org/setup/>
