import qbs
import qbs.FileInfo

Product
{
    property stringList paths: []
    Depends { name: "Qt"; submodules: [ "core" ]; }
    builtByDefault: false
    type: "qch"
    
    files:
    {
        var list = [];
        for(var i in paths)
            list.push(paths[i] + "/*.qdoc");
        return list;
    }

    Group
    {
        files:
        {
            var list = [];
            for(var i in paths)
                list.push(paths[i] + "/*.qdocconf");
            return list;
        }
        fileTags: "qdocconf-main"
    }

    Group
    {
        fileTagsFilter: ["qdoc-output"]
        qbs.install: true
        qbs.installDir: FileInfo.joinPaths(project.installDirectory, "doc")
        qbs.installSourceBase: Qt.core.qdocOutputDir
    }

    Group
    {
        fileTagsFilter: ["qch"]
        qbs.install: true
        qbs.installDir: FileInfo.joinPaths(project.installDirectory, "doc")
    }
}
