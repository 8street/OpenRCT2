/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "GuestSetNameAction.h"

#include "../Cheats.h"
#include "../Context.h"
#include "../core/MemoryStream.h"
#include "../drawing/Drawing.h"
#include "../entity/Entity.h"
#include "../interface/Window.h"
#include "../localisation/Localisation.h"
#include "../localisation/StringIds.h"
#include "../windows/Intent.h"
#include "../world/Park.h"

GuestSetNameAction::GuestSetNameAction(uint16_t spriteIndex, const std::string& name)
    : _spriteIndex(spriteIndex)
    , _name(name)
{
}

uint16_t GuestSetNameAction::GetSpriteIndex() const
{
    return _spriteIndex;
}

std::string GuestSetNameAction::GetGuestName() const
{
    return _name;
}

void GuestSetNameAction::AcceptParameters(GameActionParameterVisitor& visitor)
{
    visitor.Visit("peep", _spriteIndex);
    visitor.Visit("name", _name);
}

uint16_t GuestSetNameAction::GetActionFlags() const
{
    return GameAction::GetActionFlags() | GameActions::Flags::AllowWhilePaused;
}

void GuestSetNameAction::Serialise(DataSerialiser& stream)
{
    GameAction::Serialise(stream);

    stream << DS_TAG(_spriteIndex) << DS_TAG(_name);
}

GameActions::Result GuestSetNameAction::Query() const
{
    if (_spriteIndex >= MAX_ENTITIES)
    {
        return GameActions::Result(GameActions::Status::InvalidParameters, STR_CANT_NAME_GUEST, STR_NONE);
    }

    auto guest = TryGetEntity<Guest>(_spriteIndex);
    if (guest == nullptr)
    {
        log_warning("Invalid game command for sprite %u", _spriteIndex);
        return GameActions::Result(GameActions::Status::InvalidParameters, STR_CANT_NAME_GUEST, STR_NONE);
    }

    return GameActions::Result();
}

GameActions::Result GuestSetNameAction::Execute() const
{
    auto guest = TryGetEntity<Guest>(_spriteIndex);
    if (guest == nullptr)
    {
        log_warning("Invalid game command for sprite %u", _spriteIndex);
        return GameActions::Result(GameActions::Status::InvalidParameters, STR_CANT_NAME_GUEST, STR_NONE);
    }

    auto curName = guest->GetName();
    if (curName == _name)
    {
        return GameActions::Result();
    }

    if (!guest->SetName(_name))
    {
        return GameActions::Result(GameActions::Status::Unknown, STR_CANT_NAME_GUEST, STR_NONE);
    }

    // Easter egg functions are for guests only
    guest->HandleEasterEggName();

    gfx_invalidate_screen();

    auto intent = Intent(INTENT_ACTION_REFRESH_GUEST_LIST);
    context_broadcast_intent(&intent);

    auto res = GameActions::Result();
    res.Position = guest->GetLocation();

    return res;
}
