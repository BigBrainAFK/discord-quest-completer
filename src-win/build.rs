fn main() {
    println!("cargo:rerun-if-changed=src/main.c");
    let target = std::env::var("TARGET").unwrap();
    if target.contains("windows") {
        cc::Build::new()
            .file("src/main.c")
            .flag("/O1")
            .flag("/GS-")
            .flag("/Gy")   // COMDAT functions -> Crinkler can reorder them
            .flag("/Zl")
            .compile("runner_c");

        println!("cargo:rustc-link-arg=/ENTRY:mainEntryPoint");
        println!("cargo:rustc-link-arg=/SUBSYSTEM:WINDOWS");
        println!("cargo:rustc-link-arg=/NODEFAULTLIB");
        println!("cargo:rustc-link-arg=/COMPMODE:SLOW");

        println!("cargo:rustc-link-lib=kernel32");
        println!("cargo:rustc-link-lib=user32");
        println!("cargo:rustc-link-lib=gdi32");
    }
}
