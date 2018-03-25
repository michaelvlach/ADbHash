import qbs
import qbs.File
import qbs.FileInfo
import qbs.TextFile

Project
{
    id: autoproject

    //Common functions
    property var functions:
    {
        return {
            makePath: (function(path, subpath) { return FileInfo.joinPaths(path, subpath); }),
            getFilePath: (function(file) { return FileInfo.path(file); }),
            getFileName: (function(file) { return FileInfo.baseName(file); }),
            print: (function(message) { console.info(message); }),
            getDirectories: (function(directory) { var dirs = File.directoryEntries(directory, File.Dirs | File.NoDotAndDotDot); dirs.forEach(function(file, index, array) { array[index] = FileInfo.joinPaths(this.path, file); }, { path: directory }); return dirs; }),
            getFiles: (function(directory) { var files = File.directoryEntries(directory, File.Files); files.forEach(function(file, index, array) { array[index] = FileInfo.joinPaths(this.path, file); }, { path: directory }); return files; }),
            getKeys: (function(object) { return Object.keys(object); }),
            prependPath: (function(file, index, array) { array[index] = FileInfo.joinPaths(this.path, file); }),
            forAll: (function(object, func, context) { context.object = object; context.func = func; Object.keys(object).forEach(function(name) { this.name = name; this.func.call(this, this.object[name]); }, context); return object; }),
            callByName: (function(name) { this.name = name; this.func.call(this, this.object[name]); }),
            isValid: (function(object) { return object && Object.keys(object).length > 0; }),
            addFind: (function()
            {
                if(!Array.prototype.find)
                {
                    Object.defineProperty(Array.prototype, 'find',
                    {
                        value: function(predicate)
                        {
                            if(this === null) throw new TypeError('"this" is null or not defined');
                            if(typeof predicate !== 'function') throw new TypeError('predicate must be a function');
                            for(var k = 0; k < (Object(this).length >>> 0); k++) if(predicate.call(arguments[1], Object(this)[k], k, Object(this))) return Object(this)[k];
                            return undefined;
                        }
                    });
                }
            })
        };
    }
    //End of common functions

    Probe
    {
        id: configuration

        property var ProjectFormat:
        {
            return {
                Tree: "Tree",
                Flat: "Flat"
            };
        }
        property var DependencyMode:
        {
            return {
                Default: "Default",
                NoHeaderOnly: "NoHeaderOnly",
                Disabled: "Disabled"
            }
        }

        //-------------//
        //CONFIGURATION//
        //-------------//
        property string name: "ADbHash (autoproject)"
        property string autoprojectDirectory: ".autoproject"
        property string projectRoot: ""
        property string projectFormat: ProjectFormat.Flat
        property string dependencyMode: DependencyMode.Default
        property bool dryRun: false
        property string installDirectory: qbs.targetOS + "-" + qbs.architecture + "-" + qbs.toolchain.join("-")

        property string ignorePattern: "\\/(\\.autoproject|\\.git|external|tools)$"
        property string additionalDirectoriesPattern: "^include|adb$"
        property string cppSourcesPattern: "\\.cpp$"
        property string cppHeadersPattern: "\\.h$"
        property string cppStandardHeadersPath: "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.12.25827/include/"

        property var items:
        {
            return {
                AutoprojectApp: { pattern: "\\/.+Test\\.cpp$" },
                AutoprojectInclude: { pattern: "\\/.+\\.h$" },
                AutoprojectDoc: { pattern: "\\/(Doc|.+\\.qdocconf)$" }
            };
        }

        property var modules:
        {
            return {
                Qt: { includePath: "C:/Qt/5.10.1/msvc2017_64/include" },
                TestExtras: { includePath: "external/TestExtras" }
            };
        }
        //--------------------//
        //END OF CONFIGURATION//
        //--------------------//

        property string rootPath: ""
        property string outPath: ""
        property string cppPattern: ""
        property bool runTests: false
        property var functions: project.functions
        property string sourceDirectory: project.sourceDirectory

        configure:
        {
            functions.print(name + " @ " + Date());
            functions.print(new Array(39).join("-"));
            functions.print("Running steps...");
            functions.print("[1/11] Parsing configuration...");
            var start = Date.now();
            var root = functions.makePath(sourceDirectory, projectRoot);
            var out = functions.makePath(sourceDirectory, autoprojectDirectory);
            var cpp = cppSourcesPattern + "|" + cppHeadersPattern;
            rootPath = root;
            outPath = out;
            cppPattern = cpp;

            //Print configured variables
            functions.print("    Project root: " + rootPath);
            functions.print("    Output path: " + outPath);
            functions.print("    Install path: " + functions.makePath(qbs.installRoot, installDirectory));
            functions.print("    Output format: " + projectFormat);
            functions.print("    Dependency mode: " + dependencyMode);
            functions.print("    Items: \n        " + functions.getKeys(items).join("\n        "));
            functions.print("    Modules: \n        " + functions.getKeys(modules).join("\n        "));

            var time = Date.now() - start;
            functions.print("[1/11] Done (" + time + "ms)");
        }
    }

    Probe
    {
        id: modulescanner

        property var configurationModules: configuration.modules
        property bool runTests: configuration.runTests
        property string cppStandardHeadersPath: configuration.cppStandardHeadersPath
        property var modules: ({})
        property var standardHeaders: ({})
        property var functions: project.functions
        property string sourceDirectory: project.sourceDirectory

        configure:
        {
            function getSubmoduleName(directory, moduleName)
            {
                return (directory.startsWith(moduleName) ? directory.slice(moduleName.length) : directory).toLowerCase();
            }

            function addSubmodule(directory)
            {
                this.submodules[getSubmoduleName(functions.getFileName(directory), this.moduleName)] = { includePath: directory, files: functions.getFiles(directory) };
            }

            function getModuleDirectory(moduleName)
            {
                return configurationModules[moduleName].includePath;
            }

            function getSubmodules(directories, moduleName)
            {
                var submodules = {};
                directories.forEach(addSubmodule, { submodules: submodules, moduleName: moduleName });
                return submodules;
            }

            function getAbsolutePath(path)
            {
                return FileInfo.isAbsolutePath(path) ? path : functions.makePath(sourceDirectory, path);
            }

            function scanModule(module)
            {
                module.files = functions.getFiles(getAbsolutePath(module.includePath));
                module.submodules = getSubmodules(functions.getDirectories(module.includePath), this.name);
            }

            function scanModules(modules)
            {
                return functions.forAll(modules, scanModule, {});
            }

            function scanStandardIncludePath(path)
            {
                var files = File.directoryEntries(path, File.Files);
                files.forEach(function(element, index, array) { array[index] = functions.makePath(path.replace(cppStandardHeadersPath, ""), element); });
                var dirs = functions.getDirectories(path);

                for(var i in dirs)
                    files = files.concat(scanStandardIncludePath(dirs[i]));

                return files;
            }

            function scanStandardIncludes(path)
            {
                var files = scanStandardIncludePath(path);
                var includes = {};

                for(var i in files)
                    includes[files[i]] = true;

                return includes;
            }

            functions.print("[2/11] Scanning modules...");
            var start = Date.now();
            var scannedModules = scanModules(configurationModules);
            modules = scannedModules;
            var standardIncludes = scanStandardIncludes(cppStandardHeadersPath);
            standardHeaders = standardIncludes;

            //TEST
            if(runTests)
            {
                functions.print("    Running tests...");
                if(!modules.Qt) { functions.print("    FAIL: 'Qt' module is missing"); return; }
                if(!modules.Qt.submodules) { functions.print("    FAIL: 'Qt' module is missing 'submodules'"); return; }
                if(!modules.Qt.submodules.core) { functions.print("    FAIL: 'Qt.core' submodule is missing"); return; }
                if(!modules.Qt.submodules.core.includePath) { functions.print("    FAIL: 'Qt.core' submodule is missing 'includePath'"); return; }
                if(modules.Qt.submodules.core.includePath !== functions.makePath(modules.Qt.includePath, "QtCore")) { functions.print("    FAIL: 'Qt.core' has incorrect 'includePath' --- EXPECTED: \"" + functions.makePath(modules.Qt.includePath, "QtCore") + "\", ACTUAL: \"" + modules.Qt.submodules.core.includePath) + "\""; return; }
                if(!modules.Qt.submodules.core.files) { functions.print("    FAIL: 'Qt.core' submodule is missing files"); return; }
                if(!modules.Qt.submodules.core.files.contains(functions.makePath(modules.Qt.submodules.core.includePath, "QString"))) { functions.print("    FAIL: 'Qt.core' is missing 'QString' file"); return; }
                if(!standardHeaders["string"]) { functions.print("    FAIL: 'string' standard header is missing"); return; }
                if(!standardHeaders[functions.makePath("experimental", "forward_list")]) { functions.print("    FAIL: 'experimental/forward_list' standard header is missing"); return; }
                functions.print("    [Ok]");
            }

            var time = Date.now() - start;
            functions.print("[2/11] Done (" + time + "ms)");
        }
    }

    Probe
    {
        id: projectscanner

        property string rootPath: configuration.rootPath
        property string ignorePattern: configuration.ignorePattern
        property bool runTests: configuration.runTests
        property var rootProject: ({})
        property var functions: project.functions

        configure:
        {
            function isNotIgnored(element)
            {
                return !new RegExp(ignorePattern).test(element);
            }

            function appendSubproject(directory)
            {
                var project = getProject(directory);
                this.subprojects[project.name] = project;
            }

            function getSubprojects(directory)
            {
                var subprojects = {};
                functions.getDirectories(directory).filter(isNotIgnored).forEach(appendSubproject, { subprojects: subprojects });
                return subprojects;
            }

            function getProject(directory)
            {
                functions.print("    " + FileInfo.baseName(directory) + " (" + directory + ")");
                return {
                    name: FileInfo.baseName(directory),
                    product: {},
                    path: directory,
                    files: functions.getFiles(directory).filter(isNotIgnored),
                    subprojects: getSubprojects(directory)
                };
            }

            functions.print("[3/11] Creating projects...");
            var start = Date.now();
            var project = getProject(rootPath);
            rootProject = project;

            //TEST
            if(runTests)
            {
                functions.print("    Running tests...");
                if(functions.getKeys(rootProject).join(",") !== "name,product,path,files,subprojects") { functions.print("    FAIL: Failed to project values --- EXPECTED: \"name,product,path,files,subprojects\", ACTUAL: \"" + functions.getKeys(rootProject) + "\""); return; }
                if(rootProject.path !== rootPath) { functions.print("    FAIL: Root project path is incorrect, EXPECTED: \"" + rootPath + "\", ACTUAL: \"" + rootProject.path + "\""); return; }
                if(rootProject.name !== "Example") { functions.print("    FAIL: Root project name is incorrect --- EXPECTED: \"Example\", ACTUAL: \"" + rootProject.name + "\""); return; }
                if(!rootProject.subprojects) { functions.print("    FAIL: Root project is missing 'subprojects'"); return; }
                if(functions.getKeys(rootProject.subprojects).join(",") !== "Doc,Include,src") { functions.print("    FAIL: Failed to detect subprojects --- EXPECTED: \"Doc,Include,src\", ACTUAL: \"" + functions.getKeys(rootProject.subprojects) + "\""); return; }
                if(!rootProject.subprojects.Include.path) { functions.print("    FAIL: Project 'rootProject.subprojects.Include' is missing 'path'"); return; }
                if(rootProject.subprojects.Include.path !== functions.makePath(rootProject.path, "Include")) { functions.print("    FAIL: 'rootProject.subprojects.Include' project 'path' is incorrect --- EXPECTED: \"" + functions.makePath(rootProject.path, "Include") + "\", ACTUAL: \"" + rootProject.subprojects.Include.path + "\""); return; }
                if(!rootProject.subprojects.Include.subprojects) { functions.print("    FAIL: 'rootProject.subprojects.Include' is missing 'subprojects'"); return; }
                if(!functions.getKeys(rootProject.subprojects.Include.subprojects).contains("MyLibrary")) { functions.print("    FAIL: 'rootProject.subprojects.Include' is missing subproject 'MyLibrary'"); return; }
                if(!rootProject.subprojects.Include.subprojects.MyLibrary.files) { functions.print("    FAIL: 'rootProject.subprojects.Include.subprojects.MyLibrary' is missing 'files'"); return; }
                if(!rootProject.subprojects.Include.subprojects.MyLibrary.files.contains(functions.makePath(rootProject.subprojects.Include.subprojects.MyLibrary.path, "MyLibrary.h"))) { functions.print("    FAIL: 'rootProject.subprojects.Include.subprojects.MyLibrary' is missing files 'MyLibrary.h'"); return; }
                functions.print("    [Ok]");
            }

            var time = Date.now() - start;
            functions.print("[3/11] Done (" + time + "ms)");
        }
    }

    Probe
    {
        id: productscanner

        property var scannedRootProject: projectscanner.rootProject
        property var items: configuration.items
        property string cppPattern: configuration.cppPattern
        property bool runTests: configuration.runTests
        property var rootProject: ({})
        property var functions: project.functions

        configure:
        {
            function getItemPattern(item)
            {
                return items[item].pattern;
            }

            function getItemContentPattern(item)
            {
                return items[item].contentPattern;
            }

            function isPathContentItem(item, files)
            {
                return files && getItemContentPattern(item) ? files.some(function(file) { return isContentItem(getFileContent(file), item); }) : true;
            }

            function isDirItem(item)
            {
                return isPathItem.call({ path: this.path }, item) && isPathContentItem(item, this.files);
            }

            function isPathItem(item)
            {
                return new RegExp(getItemPattern(item)).test(this.path);
            }

            function isContentItem(content, item)
            {
                return new RegExp(getItemContentPattern(item)).test(content);
            }

            function getItemNames()
            {
                return functions.getKeys(items);
            }

            function getFileContent(file)
            {
                var textFile = new TextFile(file);
                var content = textFile.readAll();
                textFile.close();
                return content;
            }

            function getItemFromProjectPath(project)
            {
                return getItemNames().find(isDirItem, { path: project.path, files: project.files });
            }

            function isFileContentItem(file)
            {
                return !getItemContentPattern(this.item) || isContentItem(getFileContent(file), this.item);
            }

            function isFileItem(file)
            {
                return isPathItem.call({ path: file }, this.item) && isFileContentItem.call({ item: this.item }, file);
            }

            function areFilesItem(item)
            {
                return this.files && this.files.some(isFileItem, { item: item });
            }

            function getItemFromProjectFiles(files)
            {
                return getItemNames().find(areFilesItem, { files: files });
            }

            function prependParentName(parent, name)
            {
                if(parent)
                    return parent.product.name ? (parent.product.name + name) : (functions.getFileName(parent.path) + name);
                else
                    return name;
            }

            function getProduct(project, parentProject)
            {
                var name = project.name;
                var item = getItemFromProjectPath(project);

                if(!item)
                    item = getItemFromProjectFiles(project.files);
                else
                    name = prependParentName(parentProject, project.name);

                if(item)
                {
                    project.product.item = item;
                    project.product.name = name;
                    project.product.paths = [project.path];
                    project.product.files = project.files;
                    project.product.includes = {};
                    project.product.dependencies = {};
                    project.product.includePaths = {};
                    project.product.includedPaths = {};
                    project.product.headerOnly = false;
                    functions.print("    " + project.product.name + " (" + project.product.item + "): " + project.path);
                }

                return project.product;
            }

            function scanSubprojects(project)
            {
                return functions.forAll(project.subprojects, function(subproject) { scanProject(subproject, this.project) }, { project: project });
            }

            function scanProject(project, parent)
            {
                project.product = getProduct(project, parent);
                project.subprojects = scanSubprojects(project);
                return project;
            }

            functions.print("[4/11] Creating products...");
            var start = Date.now();
            functions.addFind();
            var project = scanProject(scannedRootProject, undefined);
            rootProject = project;

            //TEST
            if(runTests)
            {
                functions.print("    Running tests...");
                if(functions.getKeys(rootProject.subprojects.Include.product).join(",") !== "item,name,paths,files,includes,dependencies,includePaths,includedPaths,headerOnly") { functions.print("    FAIL: Failed to detect product values --- EXPECTED: \"item,name,paths,files,includes,dependencies,includePaths,includedPaths,headerOnly\", ACTUAL: \"" + functions.getKeys(rootProject.subprojects.Include.product) + "\""); return; }
                if(rootProject.subprojects.Include.product.name !== "ExampleInclude") { functions.print("    FAIL: Incorrect name of product 'rootProject.subprojects.Include' --- EXPECTED: \"ExampleInclude\", ACTUAL: \"" + rootProject.subprojects.Include.product.name + "\""); return; }
                if(rootProject.subprojects.Include.product.item !== "AutoprojectInclude") { functions.print("    FAIL: Incorrect item of product 'rootProject.subprojects.Include' --- EXPECTED: \"AutoprojectInclude\", ACTUAL: \"" + rootProject.subprojects.Include.product.item + "\""); return; }
                if(!rootProject.subprojects.Include.product.files.contains(functions.makePath(rootProject.subprojects.Include.path, "Common.h"))) { functions.print("    FAIL: Project 'rootProject.subprojects.Include' is missing file 'Common.h'"); return; }
                if(rootProject.subprojects.src.subprojects.apps.subprojects.Application.product.item !== "AutoprojectApp") { functions.print("    FAIL: Incorrect item of product 'rootProject.subprojects.src.subprojects.apps.subprojects.Application' --- EXPECTED: \"AutoprojectApp\", ACTUAL: \"" + rootProject.subprojects.src.subprojects.apps.subprojects.Application.product.item + "\""); return; }
                if(rootProject.subprojects.src.subprojects.apps.subprojects.Application.product.name !== "Application") { functions.print("    FAIL: Incorrect name of product 'rootProject.subprojects.src.subprojects.apps.subprojects.Application' --- EXPECTED: \"Application\", ACTUAL: \"" + rootProject.subprojects.src.subprojects.apps.subprojects.Application.product.name + "\""); return; }
                if(rootProject.subprojects.src.subprojects.apps.subprojects.Application.product.name !== "Application") { functions.print("    FAIL: Incorrect name of product 'rootProject.subprojects.src.subprojects.apps.subprojects.Application' --- EXPECTED: \"Application\", ACTUAL: \"" + rootProject.subprojects.src.subprojects.apps.subprojects.Application.product.name + "\""); return; }
                if(rootProject.subprojects.src.subprojects.libs.subprojects.Library.product.item !== "AutoprojectStaticLib") { functions.print("    FAIL: Incorrect item of product 'rootProject.subprojects.src.subprojects.libs.subprojects.Library' --- EXPECTED: \"AutoprojectStaticLib\", ACTUAL: \"" + rootProject.subprojects.src.subprojects.libs.subprojects.Library.product.item + "\""); return; }
                functions.print("    [Ok]");
            }

            var time = Date.now() - start;
            functions.print("[4/11] Done (" + time + "ms)");
        }
    }

    Probe
    {
        id: productmerger

        property var scannedRootProject: productscanner.rootProject
        property string additionalDirectoriesPattern: configuration.additionalDirectoriesPattern
        property var items: configuration.items
        property var itemNames: project.functions.getKeys(items)
        property bool runTests: configuration.runTests
        property var rootProject: ({})
        property var functions: project.functions

        configure:
        {
            function getHigherItem(item, other)
            {
                return itemNames.indexOf(item) < itemNames.indexOf(other) ? item : other;
            }

            function mergeArrays(array, other)
            {
                return array.concat(other);
            }

            function mergeProduct(project)
            {
                if(functions.isValid(project.product) && isAdditionalDirectory(project.path))
                {
                    this.product.item = getHigherItem(this.product.item, project.product.item);
                    this.product.paths = mergeArrays(this.product.paths, project.product.paths);
                    this.product.files = mergeArrays(this.product.files, project.product.files);
                    functions.print("    '" + project.name + "' (" + project.path + ") ---> '" + this.product.name + "' (" + this.product.paths[0] + ")");
                    project.product = {};
                }
            }

            function isAdditionalDirectory(directory)
            {
                return new RegExp(additionalDirectoriesPattern).test(directory);
            }

            function mergeProducts(project)
            {
                if(functions.isValid(project.product))
                    functions.forAll(project.subprojects, mergeProduct, { product: project.product })
            }

            function mergeProject(project)
            {
                functions.forAll(project.subprojects, function(subproject) { mergeProject(subproject); }, {});
                mergeProducts(project);
                return project;
            }

            functions.print("[5/11] Merging products...");
            var start = Date.now();
            var project = mergeProject(scannedRootProject);
            rootProject = project;

            //Test
            if(runTests)
            {
                functions.print("    Running tests...");
                if(functions.isValid(rootProject.subprojects.src.subprojects.libs.subprojects.Library.subprojects.include.product)) { functions.print("    FAIL: Product 'rootProject.subprojects.src.subprojects.libs.subprojects.Library.subprojects.include' was not merged with its parent"); return; }
                if(rootProject.subprojects.src.subprojects.libs.subprojects.Library.product.item !== "AutoprojectDynamicLib") { functions.print("    FAIL: 'item' of product 'rootProject.subprojects.src.subprojects.libs.subprojects.Library' was not updated, EXPECTED: \"AutoprojectDynamicLib\", \"" + rootProject.subprojects.src.subprojects.libs.subprojects.Library.product.item + "\""); return; }
                if(!rootProject.subprojects.src.subprojects.libs.subprojects.Library.product.paths.contains(rootProject.subprojects.src.subprojects.libs.subprojects.Library.subprojects.include.path)) { functions.print("    FAIL: Product 'rootProject.subprojects.src.subprojects.libs.subprojects.Library' is missing path of the merged directory 'include'"); return; }
                if(!rootProject.subprojects.src.subprojects.libs.subprojects.Library.product.files.contains(functions.makePath(rootProject.subprojects.src.subprojects.libs.subprojects.Library.subprojects.include.path, "Library.h"))) { functions.print("    FAIL: Product 'rootProject.subprojects.src.subprojects.libs.subprojects.Library' is missing file 'Library.h' from merged directory 'include'"); return; }
                functions.print("    [Ok]");
            }

            var time = Date.now() - start;
            functions.print("[5/11] Done (" + time + "ms)");
        }
    }

    Probe
    {
        id: productconsolidator
        property var mergedRootProject: productmerger.rootProject
        property string cppSourcesPattern: configuration.cppSourcesPattern
        property var items: configuration.items
        property var itemNames: project.functions.getKeys(items)
        property bool runTests: configuration.runTests
        property var rootProject: ({})
        property var functions: project.functions

        configure:
        {
            function isSourceFile(file)
            {
                return new RegExp(cppSourcesPattern).test(file);
            }

            function hasSources(proj)
            {
                return proj.product.files.some(isSourceFile);
            }

            function getHigherItem(item, other)
            {
                return itemNames.indexOf(item) < itemNames.indexOf(other) ? item : other;
            }

            function mergeArrays(array, other)
            {
                return array.concat(other);
            }

            function mergeProducts(project, other)
            {
                project.product.item = getHigherItem(project.product.item, other.product.item);
                project.product.paths = mergeArrays(project.product.paths, other.product.paths);
                project.product.files = mergeArrays(project.product.files, other.product.files);
                functions.print("    '" + other.product.name + "' (" + other.path + ") ---> '" + project.product.name + "' (" + project.path + ")");
                other.product = {};
            }

            function mergeLastTwoProjects(projects)
            {
                var leftProject = projects[projects.length - 2];
                var rightProject = projects[projects.length - 1];

                if(!hasSources(leftProject) && hasSources(rightProject))
                {
                    projects.splice(projects.length - 2, 1);
                    mergeProducts(rightProject, leftProject);
                }
                else
                {
                    projects.splice(projects.length - 1, 1);
                    mergeProducts(leftProject, rightProject);
                }
            }

            function mergeProjects(projects)
            {
                while(projects.length > 1)
                    mergeLastTwoProjects(projects);
            }

            function groupProjectsByProductName(project, projects)
            {
                if(functions.isValid(project.product))
                {
                    if(!projects[project.product.name])
                        projects[project.product.name] = [];

                    projects[project.product.name].push(project);
                }

                functions.forAll(project.subprojects, function(subproject) { groupProjectsByProductName(subproject, projects); }, {});
            }

            function consolidateProducts(project)
            {
                var projects = {};
                groupProjectsByProductName(project, projects);
                functions.forAll(projects, mergeProjects, {});
                return project;

            }

            functions.print("[6/11] Consolidating products...");
            var start = Date.now();
            var project = consolidateProducts(mergedRootProject);
            rootProject = project;

            if(runTests)
            {
                functions.print("    Running tests...");
                if(functions.isValid(rootProject.subprojects.Include.subprojects.MyLibrary.product)) { functions.print("    FAIL: Product 'rootProject.subprojects.Include.subprojects.MyLibrary' was not merged with its namesake"); return; }
                if(rootProject.subprojects.src.subprojects.libs.subprojects.MyLibrary.product.item !== "AutoprojectDynamicLib") { functions.print("    FAIL: 'item' of product 'rootProject.subprojects.src.subprojects.libs.subprojects.MyLibrary' was not updated, EXPECTED: \"AutoprojectDynamicLib\", \"" + rootProject.subprojects.src.subprojects.libs.subprojects.MyLibrary.product.item + "\""); return; }
                if(!rootProject.subprojects.src.subprojects.libs.subprojects.MyLibrary.product.paths.contains(rootProject.subprojects.Include.subprojects.MyLibrary.path)) { functions.print("    FAIL: Product 'rootProject.subprojects.src.subprojects.libs.subprojects.MyLibrary' is missing path of the merged directory"); return; }
                if(!rootProject.subprojects.src.subprojects.libs.subprojects.MyLibrary.product.files.contains(functions.makePath(rootProject.subprojects.Include.subprojects.MyLibrary.path, "MyLibrary.h"))) { functions.print("    FAIL: Product 'rootProject.subprojects.src.subprojects.libs.subprojects.MyLibrary' is missing file 'MyLibrary.h'"); return; }
                functions.print("    [Ok]");
            }

            var time = Date.now() - start;
            functions.print("[6/11] Done (" + time + "ms)");
        }
    }

    Probe
    {
        id: projectcleaner
        property var mergedRootProject: productconsolidator.rootProject
        property bool runTests: configuration.runTests
        property var rootProject: ({})
        property var functions: project.functions

        configure:
        {
            function filterProject(project)
            {
                cleanProject(project);

                if(!functions.isValid(project.product) && !functions.isValid(project.subprojects))
                {
                    functions.print("    Removed empty project '" + project.name + "' (" + project.path + ")");
                    delete this.object[this.name];
                }
            }

            function cleanProject(project)
            {
                functions.forAll(project.subprojects, filterProject, {});
                return project;
            }

            functions.print("[7/11] Cleaning projects...");
            var start = Date.now();
            var project = cleanProject(mergedRootProject);
            rootProject = project;

            if(runTests)
            {
                functions.print("    Running tests...");
                if(rootProject.subprojects.Include.subprojects.MyLibrary) { functions.print("    FAIL: Empty project 'rootProject.subprojects.Include.subprojects.MyLibrary' was not removed"); return; }
                if(rootProject.subprojects.src.subprojects.libs.subprojects.Library.subprojects.include) { functions.print("    FAIL: Empty project 'rootProject.subprojects.src.subprojects.libs.subprojects.Library.subprojects.include' was not removed"); return; }
                functions.print("    [Ok]");
            }

            var time = Date.now() - start;
            functions.print("[7/11] Done (" + time + "ms)");
        }
    }

    Probe
    {
        id: dependencyscanner
        property var cleanedRootProject: projectcleaner.rootProject
        property string cppPattern: configuration.cppPattern
        property string cppSourcesPattern: configuration.cppSourcesPattern
        property bool runTests: configuration.runTests
        property var modules: configuration.modules
        property string dependencyMode: configuration.dependencyMode
        property var rootProject: ({})
        property var includedFiles: ({})
        property var functions: project.functions

        configure:
        {
            function isCppFile(file)
            {
                return new RegExp(cppPattern).test(file);
            }

            function isSourceFile(file)
            {
                return new RegExp(cppSourcesPattern).test(file);
            }

            function scanFile(file)
            {
                var textFile = new TextFile(file);
                var content = textFile.readAll();
                textFile.close();
                var regex = /#include\s*[<|\"]([a-zA-Z\/\.]+)[>|\"]/g;
                var result = regex.exec(content);
                while(result)
                {
                    this.includes[result[1]] = true;
                    includes[result[1]] = {};
                    result = regex.exec(content);
                }
            }

            function scanProductFiles(project)
            {
                if(functions.isValid(project.product))
                {
                    project.product.files.filter(isCppFile).forEach(scanFile, { includes: project.product.includes });
                    project.product.headerOnly = !project.product.files.some(isSourceFile);
                }

                if(functions.isValid(project.product.includes))
                    functions.print("    " + project.product.name + " [" + functions.getKeys(project.product.includes).join(", ") + "]");
            }

            function scanDependencies(project)
            {
                functions.forAll(project.subprojects, scanDependencies, {});
                scanProductFiles(project);
                return project;
            }

            functions.print("[8/11] Scanning dependencies...");
            var start = Date.now();

            if(dependencyMode === configuration.DependencyMode.Disabled)
            {
                functions.print("Dependencies disabled --- skipping");
                rootProject = cleanedRootProject;
            }
            else
            {
                var includes = {};
                var project = scanDependencies(cleanedRootProject);
                rootProject = project;
                includedFiles = includes;

                if(runTests)
                {
                    functions.print("    Running tests...");
                    if(functions.getKeys(rootProject.subprojects.Include.product.includes).join(",") !== "QtPlugin,QString") { functions.print("    FAIL: Failed to extract includes from 'rootProject.subprojects.Include' project files --- EXPECTED: \"QtPlugin,QString\", ACTUAL: \"" + functions.getKeys(rootProject.subprojects.Include.product.includes).join(",") + "\""); return; }
                    if(functions.getKeys(rootProject.subprojects.src.subprojects.apps.subprojects.Application.product.includes).join(",") !== "PrintMessage.h,PluginInterface.h,QPluginLoader,QDebug") { functions.print("    FAIL: Failed to extract includes from 'rootProject.subprojects.src.subprojects.apps.subprojects.Application' project files --- EXPECTED: \"PrintMessage.h,PluginInterface.h,QPluginLoader,QDebug\", ACTUAL: \"" + functions.getKeys(rootProject.subprojects.Include.product.includes).join(",") + "\""); return; }
                    if(!functions.getKeys(includedFiles).contains("QString")) { functions.print("    FAIL: Include 'QString' not found in 'includedFiles'"); return; }
                    if(!functions.getKeys(includedFiles).contains("PrintMessage.h")) { functions.print("    FAIL: Include 'PrintMessage.h' not found in 'includedFiles'"); return; }
                    functions.print("    [Ok]");
                }
            }
            var time = Date.now() - start;
            functions.print("[8/11] Done (" + time + "ms)");
        }
    }

    Probe
    {
        id: includefinder
        property var scannedRootProject: dependencyscanner.rootProject
        property var includedFiles: dependencyscanner.includedFiles
        property bool runTests: configuration.runTests
        property string dependencyMode: configuration.dependencyMode
        property var modules: modulescanner.modules
        property var standardHeaders: modulescanner.standardHeaders
        property var includeMap: ({})
        property var rootProject: ({})
        property var functions: project.functions

        configure:
        {
            function findIncludeInModules(includeName)
            {
                for(var moduleName in modules)
                {
                    var file = modules[moduleName].files.find(isInclude, { includeName: includeName });

                    if(file)
                        return { name: moduleName };
                    else
                    {
                        for(var submoduleName in modules[moduleName].submodules)
                        {
                            file = modules[moduleName].submodules[submoduleName].files.find(isInclude, { includeName: includeName })

                            if(file)
                                return { name: moduleName + "." + submoduleName };
                        }
                    }
                }

                return undefined;
            }

            function isInclude(file)
            {
                return this.includeName.contains("/") ? file.endsWith(this.includeName) : file.endsWith("/" + this.includeName);
            }

            function findIncludeInProject(project, includeName)
            {
                if(functions.isValid(project.product))
                {
                    var file = project.product.files.find(isInclude, { includeName: includeName });

                    if(file)
                    {
                        project.product.includedPaths[functions.getFilePath(file)] = true;
                        return { path: functions.getFilePath(file), product: project.product, name: project.product.name };
                    }
                }

                return findIncludeInProjects(project.subprojects, includeName);
            }

            function findIncludeInProjects(projects, includeName)
            {
                var result = undefined;

                for(var projectName in projects)
                {
                    result = findIncludeInProject(projects[projectName], includeName);

                     if(result)
                         break;
                }

                return result;
            }

            function isStandardHeader(includeName)
            {
                return standardHeaders[includeName];
            }

            function findInclude(includeName)
            {
                if(isStandardHeader(includeName))
                {
                    functions.print("    '" + includeName + "' found in standard headers");
                    delete this.includes[includeName];
                }
                else
                {
                    this.includes[includeName] = findIncludeInProject(scannedRootProject, includeName);

                    if(!functions.isValid(this.includes[includeName]))
                        this.includes[includeName] = findIncludeInModules(includeName);

                    if(functions.isValid(this.includes[includeName]))
                        functions.print("    '" + includeName + "' found in '" + this.includes[includeName].name);
                    else if(!includeName.endsWith(".moc"))
                        functions.print("    WARNING: Dependency '" + includeName + "' not found in any project or module");
                }
            }

            function findIncludes(includes)
            {
                functions.getKeys(includes).forEach(findInclude, { includes: includes });
                return includes;
            }

            functions.print("[9/11] Finding includes...");
            var start = Date.now();

            if(dependencyMode === configuration.DependencyMode.Disabled)
            {
                functions.print("Dependencies disabled --- skipping");
                rootProject = scannedRootProject;
            }
            else
            {
                functions.addFind();
                var includes = findIncludes(includedFiles);
                includeMap = includes;
                rootProject = scannedRootProject;

                if(runTests)
                {
                    functions.print("    Running tests...");
                    if(includeMap.QString.name !== "Qt.core") { functions.print("    FAIL: Dependency for include 'QString' not found"); return; }
                    if(includeMap["PrintMessage.h"].name !== "ApplicationLib") { functions.print("    FAIL: Dependency for include 'PrintMessage.h' not found"); return; }
                    if(includeMap["PrintMessage.h"].path !== rootProject.subprojects.src.subprojects.apps.subprojects.Application.subprojects.Lib.path) { functions.print("    FAIL: Path for include 'PrintMessage.h' incorrect --- EXPECTED: \"" + rootProject.subprojects.src.subprojects.apps.subprojects.Application.subprojects.Lib.path + "\", ACTUAL: \"" + includeMap["PrintMessage.h"].path + "\""); return; }
                    if(!rootProject.subprojects.src.subprojects.apps.subprojects.Application.subprojects.Lib.product.includedPaths[rootProject.subprojects.src.subprojects.apps.subprojects.Application.subprojects.Lib.path]) { functions.print("    FAIL: 'includedPath' for include 'PrintMessage.h' not set"); return; }
                    functions.print("    [Ok]");
                }
            }

            var time = Date.now() - start;
            functions.print("[9/11] Done (" + time + "ms)");
        }
    }

    Probe
    {
        id: dependencysetter
        property var dependencyScanRootProject: includefinder.rootProject
        property var includes: includefinder.includeMap
        property bool runTests: configuration.runTests
        property string dependencyMode: configuration.dependencyMode
        property var rootProject: ({})
        property var functions: project.functions

        configure:
        {
            function setProductDependencies(project)
            {
                for(var include in project.product.includes)
                {
                    var dependency = includes[include];

                    if(functions.isValid(dependency))
                    {
                        if(dependencyMode === configuration.DependencyMode.NoHeaderOnly && dependency.product && dependency.product.headerOnly)
                            project.product.includePaths[dependency.path] = true;
                        else
                            project.product.dependencies[dependency.name] = true;
                    }
                }

                delete project.product.dependencies[project.product.name];
            }

            function setDependencies(project)
            {
                if(functions.isValid(project.product))
                {
                    setProductDependencies(project);
                    functions.print("    " + project.product.name);
                }

                functions.forAll(project.subprojects, setDependencies, {});
                return project;
            }

            functions.print("[10/11] Setting dependencies...");
            var start = Date.now();

            if(dependencyMode === configuration.DependencyMode.Disabled)
            {
                functions.print("Dependencies disabled --- skipping");
                rootProject = dependencyScanRootProject;
            }
            else
            {
                var project = setDependencies(dependencyScanRootProject);
                rootProject = project;

                if(runTests)
                {
                    functions.print("    Running tests...");
                    if(!rootProject.subprojects.src.subprojects.apps.subprojects.Application.product.dependencies["Qt.core"]) { functions.print("    FAIL: Missing dependency 'Qt.core' in 'Application' product"); return; }
                    if(!rootProject.subprojects.src.subprojects.apps.subprojects.Application.product.dependencies["ApplicationLib"]) { functions.print("    FAIL: Missing dependency 'ApplicationLib' in 'Application' product"); return; }
                    functions.print("    [Ok]");
                }
            }
            var time = Date.now() - start;
            functions.print("[10/11] Done (" + time + "ms)");

        }
    }

    Probe
    {
        id: projectwriter
        property var rootProject: dependencysetter.rootProject
        property string outPath: configuration.outPath
        property string projectFormat: configuration.projectFormat
        property bool dryRun: configuration.dryRun
        property string installDirectory: configuration.installDirectory
        property var references: []
        property var functions: project.functions

        configure:
        {
            function write(proj)
            {
                var filePath = FileInfo.joinPaths(outPath, proj["name"] + ".qbs")
                var file = new TextFile(filePath, TextFile.WriteOnly);
                file.writeLine("import qbs");
                file.writeLine("");
                file.writeLine("Project");
                file.writeLine("{");
                file.writeLine("    name: \"" + proj.name + "\"");
                file.writeLine("    property string path: \"" + proj.path + "\"");
                file.writeLine("    property string installDirectory: \"" + installDirectory + "\"");
                file.writeLine("");

                if(functions.isValid(proj.product))
                    writeProduct(file, proj.product, "    ");

                for(var subproject in proj.subprojects)
                    writeProject(file, proj.subprojects[subproject], "    ");

                file.writeLine("}");
                file.close();
                return [filePath];
            }

            function isProjectFormatFlat()
            {
                return projectFormat === configuration.ProjectFormat.Flat;
            }

            function writeProject(file, proj, indent)
            {
                if(!isProjectFormatFlat())
                {
                    file.writeLine(indent + "Project");
                    file.writeLine(indent + "{");
                    file.writeLine(indent + "    name: \"" + proj.name + "\"");
                    file.writeLine(indent + "    property string path: \"" + proj.path + "\"");
                    file.writeLine(indent + "    property string installDirectory: project.installDirectory");
                    file.writeLine(indent + "");
                }

                if(functions.isValid(proj.product))
                    writeProduct(file, proj.product, isProjectFormatFlat() ? indent : (indent + "    "));

                for(var subproject in proj.subprojects)
                    writeProject(file, proj.subprojects[subproject], isProjectFormatFlat() ? indent : (indent + "    "));

                if(projectFormat === configuration.ProjectFormat.Tree)
                    file.writeLine(indent + "}");
            }

            function writeIncludePaths(file, includedPaths, includePaths, indent)
            {
                if(includedPaths.length > 0 || includePaths.length > 0)
                {
                    file.writeLine(indent + "    Depends { name: \"cpp\" }")
                    file.writeLine(indent + "    cpp.includePaths: [\"" + includedPaths.concat(includePaths).join("\", \"") + "\"]");
                }
            }

            function writeExport(file, includedPaths, includePaths, dependencies, indent)
            {
                if(includePaths.length > 0 || includedPaths.length > 0 || dependencies.length > 0)
                {
                    file.writeLine(indent + "Export");
                    file.writeLine(indent + "{");

                    if(includePaths.length > 0 || includedPaths.length > 0)
                    {
                        file.writeLine(indent + "    Depends { name: \"cpp\" }")
                        file.writeLine(indent + "    cpp.includePaths: [\"" + includedPaths.concat(includePaths).join("\", \"") + "\"]");
                    }

                    if(dependencies.length > 0)
                        dependencies.forEach(writeDependency, { file: file, indent: indent + "    " });

                    file.writeLine(indent + "}");
                }
            }

            function writeProduct(file, product, indent)
            {
                file.writeLine(indent + product.item);
                file.writeLine(indent + "{");
                file.writeLine(indent + "    name: \"" + product.name + "\"");
                file.writeLine(indent + "    paths: [\"" + product.paths.join("\", \"") + "\"]");
                writeExport(file, functions.getKeys(product.includedPaths), functions.getKeys(product.includePaths), functions.getKeys(product.dependencies), indent + "    ");

                if(!product.headerOnly)
                    writeIncludePaths(file, functions.getKeys(product.includedPaths), functions.getKeys(product.includePaths), indent);

                functions.getKeys(product.dependencies).forEach(writeDependency, {file: file, indent: indent + "    "});
                file.writeLine(indent + "}");
                functions.print("    " + product.name);
            }

            function writeDependency(dependency)
            {
                this.file.writeLine(this.indent + "Depends { name: \"" + dependency + "\" }");
            }

            functions.print("[11/11] Writing project...");
            var start = Date.now();

            if(dryRun)
                functions.print("Dry run --- skipping");
            else
            {
                var refs = write(rootProject);
                references = refs;
            }

            var time = Date.now() - start;
            functions.print("[11/11] Done (" + time + "ms)");
        }
    }

    name: configuration.name
    qbsSearchPaths: configuration.autoprojectDirectory
    references: projectwriter.references
}
