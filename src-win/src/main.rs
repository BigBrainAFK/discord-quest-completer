/*
 * Real application and logic resides in c code
 * This is just an inert file and does nothing.
 * The real entry point is defined in build.rs via the entrypoint flag
 */
#![no_main]
#![no_std]

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}
