import qbs

CppApplication
{
    property stringList paths: []
    targetName: qbs.buildVariant == "debug" ? name + "d" : name
    
    files:
    {
        var list = [];
        for(var i in paths)
            list.push(paths[i] + "/*");
        return list;
    }
    
    Export
    {
        Depends { name: "cpp" }        
        Parameters
        {
            cpp.link: false
        }
    }
    
    Group
    {
        qbs.install: true
        qbs.installDir: project.installDirectory
        fileTagsFilter: ["application"]
    }
}
