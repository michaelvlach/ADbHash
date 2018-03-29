import qbs
import qbs.FileInfo

Product
{
    property var rootProject:
    {
        var parent = project;
        while(parent.parent)
            parent = parent.parent;
        return parent;
    }
    property stringList includePaths:
    {
        var list = ["."];
        for(var i in rootProject.autoprojectIncludePaths)
            list.push(i);
        return list;
    }
    property stringList paths: []
    Depends { name: "Qt"; submodules: [ "core" ]; }
    Qt.core.qdocEnvironment: ["INCLUDEPATHS=" + includePaths.join(" ")]
    builtByDefault: false
    type: "qch"
    
    files:
    {
        var list = [];
        for(var i in paths)
        {
            list.push(FileInfo.joinPaths(paths[i], "*.qdoc"));
            list.push(FileInfo.joinPaths(paths[i], "*.h"));
        }
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
