#include <assert.h>
#include "gui_events.h"
#include "config.h"

// Define the gui event string names.
static const char* gui_event_config_names[gui_ShortCutEventMax] =
{
#define GUI_EVENT(name) #name,
#include "gui_events.def"
#undef GUI_EVENT
};

const char* gui_event_get_name(gui_ShortCutEvent event)
{
    assert(event >= 0 && event < gui_ShortCutEventMax);
    return gui_event_config_names[event];
}

void gui_event_get_shortcut_string(char* buffer, int bufferSize, gui_ShortCutEvent shortcutEvent)
{
    assert(shortcutEvent >= 0 && shortcutEvent < gui_ShortCutEventMax);
    config_Key shortcutKey = config_shortcuts.shortcuts[shortcutEvent];

    const char* scancodeName = SDL_GetScancodeName(shortcutKey.scancode);
    const char* modifierName = gui_event_get_modifier_name(shortcutKey.modifier);
    if (scancodeName && modifierName)
    {
        snprintf(buffer, bufferSize, "%s+%s", modifierName, scancodeName);
    }
    else if (scancodeName)
    {
        snprintf(buffer, bufferSize, "%s", scancodeName);
    }
    else
    {
        buffer[0] = 0;
    }
}

const char* gui_event_get_modifier_name(SDL_Keymod modifier)
{
    if (modifier & KMOD_CTRL)
    {
        if ((modifier & KMOD_LCTRL) && !(modifier & KMOD_RCTRL))
        {
            return "LCtrl";
        }
        else if ((modifier & KMOD_RCTRL) && !(modifier & KMOD_LCTRL))
        {
            return "RCtrl";
        }

        return "Ctrl";
    }

    if (modifier & KMOD_ALT)
    {
        if ((modifier & KMOD_LALT) && !(modifier & KMOD_RALT))
        {
            return "LAlt";
        }
        else if ((modifier & KMOD_RALT) && !(modifier & KMOD_LALT))
        {
            return "RAlt";
        }

        return "Alt";
    }

    if (modifier & KMOD_SHIFT)
    {
        if ((modifier & KMOD_LSHIFT) && !(modifier & KMOD_RSHIFT))
        {
            return "LShift";
        }
        else if ((modifier & KMOD_RSHIFT) && !(modifier & KMOD_LSHIFT))
        {
            return "RShift";
        }

        return "Shift";
    }

    if (modifier & KMOD_GUI)
    {
        if ((modifier & KMOD_LGUI) && !(modifier & KMOD_RGUI))
        {
            return "L" GUI_EVENT_PLATFORM_KEY_CAMELCASE;
        }
        else if ((modifier & KMOD_RGUI) && !(modifier & KMOD_LGUI))
        {
            return "R" GUI_EVENT_PLATFORM_KEY_CAMELCASE;
        }

        return GUI_EVENT_PLATFORM_KEY_CAMELCASE;
    }

    return nullptr;
}
