use solana_program::{
    account_info::AccountInfo,
    entrypoint,
    entrypoint::ProgramResult,
    pubkey::Pubkey,
    program_error::ProgramError,
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
    let d = 30.0 / 4.0;
    let e = 2.0;
    let f = calculate_f(d, e);

    if c == (f as u64) {
        Ok(())
    } else {
        Err(ProgramError::InvalidArgument)
    }
}

fn calculate(a: u64, b: u64) -> u64 {
    let result = (a + b) * (1 + 1);
    result
}

fn calculate_f(a: f64, b: f64) -> f64 {
    let result = a * b;
    result
}