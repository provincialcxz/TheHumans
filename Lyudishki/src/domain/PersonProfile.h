#pragma once

#include <QString>

struct PersonProfile {
    int personId = 0;

    // Личность
    QString character;
    QString interests;
    QString humorAndTriggers;
    QString gender;

    // Взгляды и идентичность
    QString politicalViews;
    QString religiousViews;
    QString languages;
    QString nicknames;

    // Семья и связи
    QString family;
    QString relationshipStatus;
    QString mutualAcquaintances;
    QString howAndWhenMet;

    // Работа и навыки
    QString careerTrack;
    QString education;
    QString skills;
    QString workSchedule;
    QString workContacts;

    // Цифровой след
    QString platformAccounts;
    QString mediaConsumption;

    // Быт
    QString property;
    QString favoritePlaces;
    QString pets;

    // Здоровье
    QString healthAndAllergies;

    // Мета
    QString reliability;
    QString infoFreshness;

    // Подарки и предпочтения
    QString giftIdeas;
    QString clothingSize;
    QString favoriteFood;

    // Прочее
    QString additionalNotes;
};
