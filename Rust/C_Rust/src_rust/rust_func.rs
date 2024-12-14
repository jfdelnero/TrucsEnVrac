#[no_mangle]
pub unsafe extern "C" fn rust_func()
{
	println!("Hello from Rust !\n");
}

#[no_mangle]
pub unsafe extern "C" fn rust_func2(table : &Vec<u8>)
{
	for i in 0..table.len()
	{
		println!("{}\n",table[i]);
	}
}

#[no_mangle]
pub unsafe extern "C" fn main_t() {
	let v = vec![0, 2, 4, 6];

	for i in 1..100
	{
		println!("Hello World! {}", i );
		rust_func2(&v);
	}
}
