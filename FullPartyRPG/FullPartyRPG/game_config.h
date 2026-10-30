#pragma once

// Change character and enemy balance here while testing the game.
namespace GameConfig {
    constexpr int KnightMaxHP = 20;
    constexpr int KnightSwordDamage = 4;
    constexpr int KnightProvokeReduction = 1;
    constexpr int KnightDefendReduction = 3;

    constexpr int RangerMaxHP = 18;
    constexpr int RangerBowDamage = 5;
    constexpr int RangerHerbalismHealing = 3;
    constexpr int RangerStudyBonus = 1;

    constexpr int WizardMaxHP = 13;
    constexpr int WizardStartingMana = 0;
    constexpr int WizardStaffDamage = 3;
    constexpr int WizardDarkblastCost = 2;
    constexpr int WizardDarkblastDamage = 7;
    constexpr int WizardJumpingSparkCost = 3;
    constexpr int WizardJumpingSparkDamage = 4;
    constexpr int WizardChillingAirCost = 3;
    constexpr int WizardManaPerTurn = 1;

    constexpr int BardMaxHP = 15;
    constexpr int BardSongAmount = 2;
    constexpr int BardDaggerDamage = 3;

    constexpr int SkeletonMaxHP = 15;
    constexpr int SkeletonAttackDamage = 3;
}
