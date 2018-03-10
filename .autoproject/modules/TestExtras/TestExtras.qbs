import qbs

Module
{
    Depends { name: "cpp" }
    Depends { name: "Qt.testlib" }
    cpp.includePaths: "../external/TestExtras/"
}
