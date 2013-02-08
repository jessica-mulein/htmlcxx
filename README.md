# About

A fork of htmlcxx with Visual C++ support. Contains only html parser. The Source files taken from [github.com/dhoerl/htmlcxx](https://github.com/dhoerl/htmlcxx) that is based on [htmlcxx.sourceforge.net](http://htmlcxx.sourceforge.net/).

# License

[http://htmlcxx.sourceforge.net](http://htmlcxx.sourceforge.net/)

# Changed

**16.12.2012**

Changes based on source code from [github.com/boazy/htmlcxx](https://github.com/boazy/htmlcxx)

- Added export_api.h;
- Add "DLL-build config" VS 2010 project.

**14.12.2012**

- New folder structure:
	- include - for exported headers;
	- build - platform dependent scripts and projects for builging;
	- bin - for build shared libraries;
	- lib - for build static libraries;
- Added folder include/impl that contains 'not for export' source code but needed for exported headers;
- Renamed files: ParserSax.tcc to ParserSax\_.h, wincstring.h to fix\_win\_cstring.h;
- VS 2010 project for static libs;
- Fixed bug with VS character functions: `(char)-10 -> (int)-10 -> (unsigned)BIG`. See [more](http://connect.microsoft.com/VisualStudio/feedback/details/646050/prolem-with-chvalidator-in-ictype-c).
- Added fix\_char.h (locale independent version of isspace + type of param is unsigned char, see fix above);
- Fixed undefined behavior in utils.cc;
- Removed test files add files of CSS parser.

# TODO

- Add CSS parser;
- Add Tests.