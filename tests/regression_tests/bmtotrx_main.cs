//
//
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace bmtotrx
{
    public class ProxyGenerator : System.IDisposable
    {
        string out_dir;
        string class_name = "basin_modeling_regression_tests_proxy";
        System.IO.StreamWriter code;

        public ProxyGenerator(string out_dir)
        {
            this.out_dir = out_dir;
        }

        public string escape_function_name(string test_name)
        {
            string escaped = test_name.Replace('-', '_');
            return escaped;
        }

        public void create_header()
        {
            code = System.IO.File.CreateText(System.IO.Path.Combine(this.out_dir, this.class_name + ".cs"));
            code.WriteLine("using System;");
            code.WriteLine("using Microsoft.VisualStudio.TestTools.UnitTesting;");
            code.WriteLine("namespace unit_tests");
            code.WriteLine("{");
            code.WriteLine("[TestClass()]");
            code.WriteLine("public class {0} {{", this.class_name);
            code.WriteLine(
        @"
        private TestContext testContextInstance;
        public TestContext TestContext
        {
            get
            {
                return testContextInstance;
            }
            set
            {
                testContextInstance = value;
            }
        }"
        );

        }

        public void create_footer()
        {
            code.WriteLine("}} // end of class {0}", this.class_name);
            code.WriteLine("} // end of namespace");
            code.Close();
        }

        void method_header(string method_name)
        {
            code.WriteLine("\t\t[TestMethod()]");
            code.WriteLine("\t\tpublic void {0}() {{", escape_function_name(method_name));
        }

        void method_footer()
        {
            code.WriteLine("\t\t}");
        }

        public void create_succeeded_method(string test_name, string succeeded_filename, string result_filename)
        {
            method_header(test_name);
            code.WriteLine("\t\t\t// Sehr gut, Voldemar!");
            method_footer();
        }

        public void create_failed_method(string test_name, string failed_filename, string result_filename)
        {
            method_header(test_name);
            code.WriteLine("\t\t\tAssert.Fail(\"{0}\");", test_name);
            method_footer();
        }

        void IDisposable.Dispose()
        {
            if (code != null)
                code.Dispose();
        }
    }

    class Program
    {
        static string replace_extension(string filename, string suffix)
        {
            string path = System.IO.Path.GetDirectoryName(filename);
            string basename = System.IO.Path.GetFileNameWithoutExtension(filename);

            return System.IO.Path.Combine(path, basename + suffix);
        }

        static void usage(string me)
        {
            Console.WriteLine("Usage: {0} test-result-directory output-code-directory", me);
            Environment.Exit(1);
        }

        static void Main(string[] args)
        {
            if(args.Length != 2) {
                usage("bmtotrx");
            }

            var test_result_dir = args[0];
            var output_code_directory = args[1];

            var apps_dirs = new List<string>(System.IO.Directory.EnumerateDirectories(test_result_dir, "*", System.IO.SearchOption.AllDirectories));
            Console.WriteLine("log_files.Count() = {0}", apps_dirs.Count());
            for (int i = 0; i != apps_dirs.Count; ++i)
            {
                Console.WriteLine("{0} -> {1}", i, apps_dirs[i]);
            }

            var log_files = new List<string>(System.IO.Directory.EnumerateFiles(test_result_dir, "*.log", System.IO.SearchOption.AllDirectories));
            Console.WriteLine("log_files.Count() = {0}", log_files.Count());

            using (var coder = new ProxyGenerator(output_code_directory))
            {
                coder.create_header();

                for (int i = 0; i != log_files.Count; ++i)
                {
                    var result_filename = replace_extension(log_files[i], ".result");
                    var succeeded_filename = replace_extension(log_files[i], ".succeeded");
                    var failed_filename = replace_extension(log_files[i], ".failed");
                    var test_name = System.IO.Path.GetFileNameWithoutExtension(log_files[i]);

                    if (System.IO.File.Exists(succeeded_filename))
                        coder.create_succeeded_method(test_name, succeeded_filename, result_filename);
                    else if (System.IO.File.Exists(failed_filename))
                        coder.create_failed_method(test_name, failed_filename, result_filename);
                    else
                        Console.WriteLine("{0} UNDEF", test_name);
                }

                coder.create_footer();
            }
        }
    }
}
