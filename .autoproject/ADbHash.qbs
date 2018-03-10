import qbs

Project
{
    name: "ADbHash"
    property string path: "C:/dev/Projects/ADbHash"
    property string installDirectory: "windows-x86_64-msvc"

    Project
    {
        name: "include"
        property string path: "C:/dev/Projects/ADbHash/include"
        property string installDirectory: project.installDirectory
    
        AutoprojectInclude
        {
            name: "include"
            paths: ["C:/dev/Projects/ADbHash/include"]
        }
    }
    Project
    {
        name: "src"
        property string path: "C:/dev/Projects/ADbHash/src"
        property string installDirectory: project.installDirectory
    
        AutoprojectInclude
        {
            name: "src"
            paths: ["C:/dev/Projects/ADbHash/src"]
        }
    }
    Project
    {
        name: "test"
        property string path: "C:/dev/Projects/ADbHash/test"
        property string installDirectory: project.installDirectory
    
        Project
        {
            name: "ADbHashTest"
            property string path: "C:/dev/Projects/ADbHash/test/ADbHashTest"
            property string installDirectory: project.installDirectory
        
            AutoprojectApp
            {
                name: "ADbHashTest"
                paths: ["C:/dev/Projects/ADbHash/test/ADbHashTest"]
            }
        }
    }
}
