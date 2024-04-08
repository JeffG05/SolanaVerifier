use borsh::{BorshDeserialize, BorshSerialize};
use solana_program::{
    account_info::{next_account_info, AccountInfo},
    entrypoint::ProgramResult,
    msg,
    program::{invoke, invoke_signed},
    program_error::ProgramError,
    pubkey::Pubkey,
    rent::Rent,
    system_instruction,
    sysvar::Sysvar,
};

use crate::{Wallet, WalletInstruction, WALLET_LEN};

pub fn process_instruction(
    program_id: &Pubkey,
    accounts: &[AccountInfo],
    mut instruction_data: &[u8],
) -> ProgramResult {
    match WalletInstruction::deserialize(&mut instruction_data)? {
        WalletInstruction::Initialize => initialize(program_id, accounts),
        WalletInstruction::Deposit { amount } => deposit(program_id, accounts, amount),
        WalletInstruction::Withdraw { amount } => withdraw(program_id, accounts, amount),
    }
}

fn initialize(program_id: &Pubkey, accounts: &[AccountInfo]) -> ProgramResult {
    msg!("init");
    let account_info_iter = &mut accounts.iter();
    let wallet_info = next_account_info(account_info_iter)?;
    let authority__signer = next_account_info(account_info_iter)?;
    let rent_info = next_account_info(account_info_iter)?;
    let (wallet_address, wallet_seed) =
        Pubkey::find_program_address(&[&authority__signer.key.to_bytes()], program_id);
    let rent = Rent::from_account_info(rent_info)?;

    assert_eq!(*wallet_info.key, wallet_address);
    assert!(wallet_info.data_is_empty());
    assert!(authority__signer.is_signer, "authority must sign!");

    invoke_signed(
        &system_instruction::create_account(
            &authority__signer.key,
            &wallet_address,
            rent.minimum_balance(WALLET_LEN as usize),
            WALLET_LEN,
            &program_id,
        ),
        &[authority__signer.clone(), wallet_info.clone()],
        &[&[&authority__signer.key.to_bytes(), &[wallet_seed]]],
    )?;

    let wallet = Wallet {
        authority: *authority__signer.key,
    };

    wallet
        .serialize(&mut &mut (*wallet_info.data).borrow_mut()[..])
        .unwrap();

    Ok(())
}

fn deposit(program_id: &Pubkey, accounts: &[AccountInfo], amount: u64) -> ProgramResult {
    msg!("deposit {}", amount);
    let account_info_iter = &mut accounts.iter();
    let wallet_info__owner = next_account_info(account_info_iter)?;
    let source_info__signer = next_account_info(account_info_iter)?;

    assert_eq!(wallet_info__owner.owner, program_id);

    invoke(
        &system_instruction::transfer(&source_info__signer.key, &wallet_info__owner.key, amount),
        &[wallet_info__owner.clone(), source_info__signer.clone()],
    )?;

    Ok(())
}

fn withdraw(program_id: &Pubkey, accounts: &[AccountInfo], amount: u64) -> ProgramResult {
    msg!("withdraw {}", amount);
    let account_info_iter = &mut accounts.iter();
    let wallet_info__owner = next_account_info(account_info_iter)?;
    let authority_info__signer = next_account_info(account_info_iter)?;
    let destination_info = next_account_info(account_info_iter)?;
    let rent_info = next_account_info(account_info_iter)?;

    let wallet = Wallet::deserialize(&mut &(*wallet_info__owner.data).borrow_mut()[..])?;
    let rent = Rent::from_account_info(rent_info)?;

    assert_eq!(wallet_info__owner.owner, program_id);
    assert_eq!(wallet.authority, *authority_info__signer.key);
    assert!(authority_info__signer.is_signer, "authority must sign!");

    let min_balance = rent.minimum_balance(WALLET_LEN as usize);
    if min_balance + amount > **wallet_info__owner.lamports.borrow_mut() {
        return Err(ProgramError::InsufficientFunds);
    }

    **wallet_info__owner.lamports.borrow_mut() -= amount;
    **destination_info.lamports.borrow_mut() += amount;

    wallet
        .serialize(&mut &mut (*wallet_info__owner.data).borrow_mut()[..])
        .unwrap();

    Ok(())
}
