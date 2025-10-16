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
                foreach (var p in Directory.GetFiles(inDir, "*.bmd", SearchOption.AllDirectories))
                    sources.Add(p);
                // If you also want .bdl, uncomment the next line.
                // sources.AddRange(Directory.GetFiles(inDir, "*.bdl", SearchOption.AllDirectories));

                if (sources.Count == 0)
                {
                    Console.WriteLine("No .bmd files found.");
                    return 0;
                }

                int ok = 0, fail = 0;

                foreach (var src in sources)
                {
                    string relPath = GetRelativePath(inDir, src);
                    string relDir = Path.GetDirectoryName(relPath) ?? string.Empty;

                    string dstDir = Path.Combine(outDir, relDir);
                    Directory.CreateDirectory(dstDir);

                    string dstFile = Path.GetFileNameWithoutExtension(src) + ".fbx";
                    string dst = Path.Combine(dstDir, dstFile);

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
                // Build args for SuperBMDLib
                var libArgs = new Arguments(new[] { src, dst, "--exportfbx" });

                var model = Model.Load(libArgs, mat_presets: null, additionalTexPath: null);

                // Adjust this if your ExportSettings signature differs in your fork
                var settings = new ExportSettings(false);

                // Export as FBX
                model.ExportAssImp(dst, "fbx", settings, libArgs);

                return File.Exists(dst);
            }
            catch (Exception e)
            {
                Console.WriteLine("Error: " + e.Message);
                return false;
            }
        }

        private static string GetRelativePath(string baseDir, string fullPath)
        {
            if (string.IsNullOrEmpty(baseDir)) throw new ArgumentNullException(nameof(baseDir));
            if (string.IsNullOrEmpty(fullPath)) throw new ArgumentNullException(nameof(fullPath));

            baseDir = Path.GetFullPath(baseDir);
            fullPath = Path.GetFullPath(fullPath);

            if (!baseDir.EndsWith(Path.DirectorySeparatorChar.ToString()) &&
                !baseDir.EndsWith(Path.AltDirectorySeparatorChar.ToString()))
            {
                baseDir += Path.DirectorySeparatorChar;
            }

            var baseUri = new Uri(baseDir, UriKind.Absolute);
            var pathUri = new Uri(fullPath, UriKind.Absolute);
            var relUri = baseUri.MakeRelativeUri(pathUri);
            var rel = Uri.UnescapeDataString(relUri.ToString());
            return rel.Replace('/', Path.DirectorySeparatorChar);
        }
    }
}
