import qbs

Product
{
    property stringList paths: []    

    files:
    {
        var list = [];
        for(var i in paths)
            list.push(paths[i] + "/*");
        return list;
    }
}
