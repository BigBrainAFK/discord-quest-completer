fn main() {
    println!("cargo:rerun-if-changed=src/main.c");
    let target = std::env::var("TARGET").unwrap();
    if target.contains("windows") {
        cc::Build::new()
            .file("src/main.c")
            .flag("/GS-")   // no stack security cookies (no __security_check_cookie)
            .flag("/O1")
            .flag("/Gy")    // function-level linking so /OPT:REF can strip dead code
            .flag("/Zl")    // omit default-library records from the .obj
            .compile("runner_c");

        println!("cargo:rustc-link-arg=/NODEFAULTLIB");
        println!("cargo:rustc-link-arg=/ENTRY:mainEntryPoint");
        println!("cargo:rustc-link-arg=/SUBSYSTEM:WINDOWS");

        println!("cargo:rustc-link-arg=/MERGE:.rdata=.text");
        println!("cargo:rustc-link-arg=/MERGE:.data=.text");
        println!("cargo:rustc-link-arg=/SECTION:.text,EWR");

        println!("cargo:rustc-link-arg=/ALIGN:16");
        println!("cargo:rustc-link-arg=/FILEALIGN:16");

        println!("cargo:rustc-link-arg=/OPT:REF");   // drop unreferenced code/data
        println!("cargo:rustc-link-arg=/OPT:ICF");   // fold identical functions
        println!("cargo:rustc-link-arg=/FIXED");     // remove the .reloc section (no ASLR)
        println!("cargo:rustc-link-arg=/DEBUG:NONE");

        println!("cargo:rustc-link-lib=kernel32");
        println!("cargo:rustc-link-lib=user32");
        println!("cargo:rustc-link-lib=gdi32");
    }
}
