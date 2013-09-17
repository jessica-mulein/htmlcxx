Changes
=======

### 16.12.2012

Changes based on [github.com/boazy/htmlcxx](https://github.com/boazy/htmlcxx)

- Added export_api.h;
- Added "DLL-build config" VS2010 project.

### 14.12.2012

- New folder structure:
    - include - for exported headers;
    - build - platform dependent scripts and projects for building;
    - bin - for build shared libraries;
    - lib - for build static libraries;
- Renamed files: ParserSax.tcc to ParserSax\_.h, wincstring.h to fix\_win\_cstring.h;
- Added folder include/impl that contains 'not for export' source code but needed for exported headers;
- Added VS2010 project for static libs;
- Added fix\_char.h;
- Fixed bug(?) in VS character functions: `(char)-10 -> (int)-10 -> (unsigned)BIG`. See [more](http://connect.microsoft.com/VisualStudio/feedback/details/646050/prolem-with-chvalidator-in-ictype-c).
- Fixed undefined behavior in utils.cc;
- Removed tests and CSS parser.
