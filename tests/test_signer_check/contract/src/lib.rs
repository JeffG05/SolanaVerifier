use borsh::{BorshDeserialize, BorshSerialize};
use solana_program::{
    account_info::{next_account_info, AccountInfo},
    entrypoint,
    entrypoint::ProgramResult,
    program_error::ProgramError,
    pubkey::Pubkey,
};

#[derive(BorshSerialize, BorshDeserialize)]
pub struct ConfigAccount {
    pub admin: Pubkey,
}

#[cfg(not(feature = "exclude_entrypoint"))]
entrypoint!(process_instruction);

fn update_admin(program_id: &Pubkey, accounts: &[AccountInfo]) -> ProgramResult {
    let account_iter = &mut accounts.iter();
    let admin__signer = next_account_info(account_iter)?;
    let config = next_account_info(account_iter)?;
    let mut config_account = ConfigAccount::try_from_slice(&config.data.borrow())?;
    let new_admin = next_account_info(account_iter)?;

    if config.owner != program_id {
        return Err(ProgramError::IncorrectProgramId);
    }

    if config_account.admin != *admin__signer.key {
        return Err(ProgramError::InvalidAccountOwner);
    }

    config_account.admin = *new_admin.key;
    config_account.serialize(&mut &mut config.data.borrow_mut()[..])?;

    Ok(())
}

pub fn process_instruction(
    program_id: &Pubkey,
    accounts: &[AccountInfo],
    instruction_data: &[u8],
) -> ProgramResult {
    update_admin(program_id, accounts)
}