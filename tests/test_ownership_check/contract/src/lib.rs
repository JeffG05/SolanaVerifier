use borsh::{BorshDeserialize, BorshSerialize};
use solana_program::{
    account_info::{next_account_info, AccountInfo},
    entrypoint::ProgramResult,
    program_error::ProgramError,
    pubkey::Pubkey,
};

#[derive(BorshSerialize, BorshDeserialize)]
pub struct ConfigAccount {
    pub admin: Pubkey,
}

#[derive(BorshSerialize, BorshDeserialize)]
pub struct ProgramInstructionData {
    pub withdraw_amount: u64,
}

#[cfg(not(feature = "exclude_entrypoint"))]
entrypoint!(process_instruction);

fn withdraw_token_restricted(program_id: &Pubkey, accounts: &[AccountInfo], amount: u64) -> ProgramResult {
    let account_iter = &mut accounts.iter();
    let vault = next_account_info(account_iter)?;
    let admin = next_account_info(account_iter)?;
    let config__owner = next_account_info(account_iter)?;
    let config_account = ConfigAccount::try_from_slice(&config__owner.data.borrow())?;
    let vault_authority = next_account_info(account_iter)?;

    if config_account.admin != *admin.key {
        return Err(ProgramError::InvalidAccountOwner);
    }

    // ...
    // Transfer funds from vault to admin using vault_authority
    // ...

    Ok(())
}

pub fn process_instruction(
    program_id: &Pubkey,
    accounts: &[AccountInfo],
    instruction_data: &[u8],
) -> ProgramResult {
    let program_instruction_data = ProgramInstructionData::try_from_slice(instruction_data)?;
    withdraw_token_restricted(program_id, accounts, program_instruction_data.withdraw_amount)
}