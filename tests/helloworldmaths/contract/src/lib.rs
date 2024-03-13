use solana_program::{
    account_info::AccountInfo,
    entrypoint,
    entrypoint::ProgramResult,
    pubkey::Pubkey,
};

// declare and export the program's entrypoint
entrypoint!(process_instruction);

// program entrypoint's implementation
pub fn process_instruction(
    _program_id: &Pubkey,
    _accounts: &[AccountInfo],
    _instruction_data: &[u8]
) -> ProgramResult {
    let a = 5;
    let b = 3;
    let c = calculate(a, b);
    let d = 30.758 / 4.5;
    let e = 2.1;
    let f = calculate_f(d, e);

    Ok(())
}

fn calculate(a: u64, b: u64) -> u64 {
    let result = (a + b) * (1 + 1);
    result
}

fn calculate_f(a: f64, b: f64) -> f64 {
    let result = a * b;
    result
}