use borsh::{BorshDeserialize, BorshSerialize};
use solana_program::{
    account_info::{next_account_info, AccountInfo},
    entrypoint,
    program_error::ProgramError,
    pubkey::Pubkey,
};

/// The type of state managed by this program. The type defined here
/// much match the `GreetingAccount` type defined by the client.
#[derive(BorshSerialize, BorshDeserialize, Debug)]
pub struct GreetingAccount {
    /// The number of greetings that have been sent to this account.
    pub counter: i32,
}

/// Declare the programs entrypoint. The entrypoint is the function
/// that will get run when the program is executed.
#[cfg(not(feature = "exclude_entrypoint"))]
entrypoint!(process_instruction);

/// Logic that runs when the program is executed. This program expects
/// a single account that is owned by the program as an argument and
/// no instructions.
///
/// The account passed in ought to contain a `GreetingAccount`. This
/// program will increment the `counter` value in the
/// `GreetingAccount` when executed.
pub fn process_instruction(
    program_id: &Pubkey,
    accounts: &[AccountInfo],
    instruction_data: &[u8],
) -> entrypoint::ProgramResult {
    // Get the account that stores greeting count information.
    let accounts_iter = &mut accounts.iter();
    let account__owner = next_account_info(accounts_iter)?;

    // The account must be owned by the program in order for the
    // program to write to it. If that is not the case then the
    // program has been invoked incorrectly and we report as much.
    // if account__owner.owner != program_id {
    //     return Err(ProgramError::IncorrectProgramId);
    // }

    // Deserialize the greeting information from the account, modify
    // it, and then write it back.
    let mut greeting = GreetingAccount::try_from_slice(&account__owner.data.borrow())?;
    if greeting.counter <= i32::MAX - 1 && greeting.counter > i32::MIN {
        greeting.counter = -greeting.counter;
    } else if greeting.counter >= i32::MAX {
        greeting.counter /= 2;
    } else {
        greeting.counter += 203;
    }
    greeting.serialize(&mut &mut account__owner.data.borrow_mut()[..])?;
    Ok(())
}