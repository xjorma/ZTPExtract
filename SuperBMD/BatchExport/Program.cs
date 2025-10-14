// File: Program.cs

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;

using SuperBMDLib;

namespace BatchExport
{
    internal static class Program
    {
        static int Main(string[] args)
        {
            try
            {
                CultureInfo.CurrentCulture = CultureInfo.InvariantCulture;

                if (args.Length != 2)
                {
                    Console.WriteLine("Usage: BatchExport <input_dir> <output_dir>.");
                    return 1;
                }

                string inDir = Path.GetFullPath(args[0]);
                string outDir = Path.GetFullPath(args[1]);

                if (!Directory.Exists(inDir))
                {
                    Console.Error.WriteLine("Input directory not found: " + inDir);
                    return 2;
                }
                Directory.CreateDirectory(outDir);

                var sources = new List<string>();
                foreach (var p in Directory.GetFiles(inDir, "*.*", SearchOption.AllDirectories))
                {
                    string ext = Path.GetExtension(p).ToLowerInvariant();
                    if (ext == ".bmd" || ext == ".bdl") sources.Add(p);
                }

                if (sources.Count == 0)
                {
                    Console.WriteLine("No .bmd or .bdl files found.");
                    return 0;
                }

                var usedNames = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                int ok = 0, fail = 0;

                foreach (var src in sources)
                {
                    string baseName = Path.GetFileNameWithoutExtension(src);
                    string outName = MakeUnique(baseName + ".fbx", usedNames, outDir);
                    string dst = Path.Combine(outDir, outName);

                    Console.WriteLine("Converting: " + src);

                    bool success = ConvertWithSuperBmdLib(src, dst);
                    if (success)
                    {
                        ok++;
                        Console.WriteLine("OK: " + dst);
                    }
                    else
                    {
                        fail++;
                        Console.WriteLine("FAIL: " + src);
                    }
                }

                Console.WriteLine("Done. Success: " + ok + ", Failed: " + fail + ".");
                return fail == 0 ? 0 : 3;
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("Fatal error: " + ex.Message);
                return 99;
            }
        }

        private static bool ConvertWithSuperBmdLib(string src, string dst)
        {
            try
            {
                // Minimal args, input then output
                var libArgs = new Arguments(new[] { src, dst, "--exportfbx" });

                var model = Model.Load(libArgs, mat_presets: null, additionalTexPath: null);

                // Your SuperBMDLib constructor requires a bool
                var settings = new ExportSettings(false);

                // Export to FBX
                model.ExportAssImp(dst, "fbx", settings, libArgs);

                return File.Exists(dst);
            }
            catch (Exception e)
            {
                Console.WriteLine("Error: " + e.Message);
                return false;
            }
        }

        private static string MakeUnique(string fileName, HashSet<string> used, string outDir)
        {
            string name = fileName;
            string stem = Path.GetFileNameWithoutExtension(fileName);
            string ext = Path.GetExtension(fileName);
            int i = 1;

            while (used.Contains(name) || File.Exists(Path.Combine(outDir, name)))
            {
                name = stem + "_" + i + ext;
                i++;
            }

            used.Add(name);
            return name;
        }
    }
}
