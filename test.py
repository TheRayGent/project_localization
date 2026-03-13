def get_attributes():
    with open(
        'D:/SteamLibrary/steamapps/common/Deadlock/game/citadel/resource/localization/citadel_attributes/citadel_attributes_russian.txt',
        encoding='utf8',
    ) as f:
        file = f.readlines()
    file = [
        i
        for i in file
        if '\t\t' in i
        and '//' not in i
        and 'modifier' not in i.lower()
        and 'StatusEffect' not in i
    ]
    with open('attributes_result.txt', encoding='utf8', mode='w') as f:
        f.writelines(file)


def get_mods():
    with open(
        'D:/SteamLibrary/steamapps/common/Deadlock/game/citadel/resource/localization/citadel_mods/citadel_mods_russian.txt',
        encoding='utf8',
    ) as f:
        file = f.readlines()
    file = [i for i in file if '"' in i and '//' not in i]
    with open('mods_result.txt', encoding='utf8', mode='w') as f:
        f.writelines(file)


def get_heroes():
    with open(
        'D:/SteamLibrary/steamapps/common/Deadlock/game/citadel/resource/localization/citadel_heroes/citadel_heroes_russian.txt',
        encoding='utf8',
    ) as f:
        file = f.readlines()
    file = [i for i in file if 'desc' in i and '//' not in i]
    print(file)
    with open('heroes_result.txt', encoding='utf8', mode='w') as f:
        f.writelines(file)


if __name__ == '__main__':
    get_attributes()
    get_mods()
    get_heroes()
