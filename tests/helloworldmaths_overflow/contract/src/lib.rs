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
    
    // gracefully exit the program
    Ok(())
}

fn calculate(a: u64, b: u64) -> u64 {
    let result = (a + b) * 3000000000000000000;
    result
}
