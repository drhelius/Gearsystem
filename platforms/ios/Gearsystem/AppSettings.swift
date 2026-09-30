import Foundation

enum GearsystemSystemOption: Int, CaseIterable {
    case automatic
    case masterSystem
    case gameGear2ASIC
    case gameGear2ASICSMS
    case gameGear1ASIC
    case gameGear1ASICSMS
    case sg1000
    case sg1000II

    var title: String {
        switch self {
        case .automatic:
            return L10n("Settings::Automatic")
        case .masterSystem:
            return "Master System / Mark III"
        case .gameGear2ASIC:
            return "Game Gear (2 ASIC)"
        case .gameGear2ASICSMS:
            return "Game Gear (2 ASIC) SMS Mode"
        case .gameGear1ASIC:
            return "Game Gear (1 ASIC)"
        case .gameGear1ASICSMS:
            return "Game Gear (1 ASIC) SMS Mode"
        case .sg1000:
            return "SG-1000 / Multivision"
        case .sg1000II:
            return "SG-1000 II"
        }
    }

    var summaryTitle: String {
        switch self {
        case .automatic:
            return title
        case .masterSystem:
            return "Master System"
        case .gameGear2ASIC, .gameGear1ASIC:
            return "Game Gear"
        case .gameGear2ASICSMS, .gameGear1ASICSMS:
            return "Game Gear SMS"
        case .sg1000:
            return "SG-1000"
        case .sg1000II:
            return "SG-1000 II"
        }
    }
}

enum GearsystemRegionOption: Int, CaseIterable {
    case automatic
    case masterSystemJapan
    case masterSystemExport
    case gameGearJapan
    case gameGearExport
    case gameGearInternational

    var title: String {
        switch self {
        case .automatic:
            return L10n("Settings::Automatic")
        case .masterSystemJapan:
            return "Master System Japan"
        case .masterSystemExport:
            return "Master System Export"
        case .gameGearJapan:
            return "Game Gear Japan"
        case .gameGearExport:
            return "Game Gear Export"
        case .gameGearInternational:
            return "Game Gear International"
        }
    }
}

enum GearsystemMapperOption: Int, CaseIterable {
    case automatic
    case rom
    case sega
    case codemasters
    case korean
    case sg1000
    case msx
    case janggun
    case korean2000XOR1F
    case koreanMSX32KB2000
    case koreanMSXSMS8000
    case koreanSMS32KB2000
    case koreanMSX8KB0300
    case korean0000XORFF
    case koreanFFFFHiCom
    case koreanFFFE
    case koreanBFFC
    case koreanFFF3FFFC
    case koreanMDFFF5
    case koreanMDFFF0
    case jumboDahjee
    case eeprom93C46
    case multi4PAKAllAction
    case iratahack

    var title: String {
        switch self {
        case .automatic: return L10n("Settings::Automatic")
        case .rom: return "ROM"
        case .sega: return "SEGA"
        case .codemasters: return "Codemasters"
        case .korean: return "Korean"
        case .sg1000: return "SG-1000"
        case .msx: return "MSX"
        case .janggun: return "Janggun"
        case .korean2000XOR1F: return "Korean 2000 XOR 1F"
        case .koreanMSX32KB2000: return "Korean MSX 32KB 2000"
        case .koreanMSXSMS8000: return "Korean MSX SMS 8000"
        case .koreanSMS32KB2000: return "Korean SMS 32KB 2000"
        case .koreanMSX8KB0300: return "Korean MSX 8KB 0300"
        case .korean0000XORFF: return "Korean 0000 XOR FF"
        case .koreanFFFFHiCom: return "Korean FFFF HiCom"
        case .koreanFFFE: return "Korean FFFE"
        case .koreanBFFC: return "Korean BFFC"
        case .koreanFFF3FFFC: return "Korean FFF3 FFFC"
        case .koreanMDFFF5: return "Korean MD FFF5"
        case .koreanMDFFF0: return "Korean MD FFF0"
        case .jumboDahjee: return "Jumbo Dahjee"
        case .eeprom93C46: return "EEPROM 93C46"
        case .multi4PAKAllAction: return "Multi 4PAK All Action"
        case .iratahack: return "Iratahack"
        }
    }
}

enum GearsystemTimingOption: Int, CaseIterable {
    case automatic
    case ntsc
    case pal

    var title: String {
        switch self {
        case .automatic: return L10n("Settings::Automatic")
        case .ntsc: return "NTSC (60 Hz)"
        case .pal: return "PAL (50 Hz)"
        }
    }
}

enum GearsystemOverscanOption: Int, CaseIterable {
    case disabled
    case topBottom
    case full284
    case full320

    var title: String {
        switch self {
        case .disabled: return L10n("Settings::Disabled")
        case .topBottom: return "Top + Bottom"
        case .full284: return "Full (284 width)"
        case .full320: return "Full (320 width)"
        }
    }
}

enum GearsystemHideLeftBarOption: Int, CaseIterable {
    case no
    case automatic
    case always

    var title: String {
        switch self {
        case .no: return L10n("Settings::No")
        case .automatic: return L10n("Settings::Automatic")
        case .always: return L10n("Settings::Always")
        }
    }
}

enum GearsystemGlassesOption: Int, CaseIterable {
    case bothEyes
    case leftEye
    case rightEye

    var title: String {
        switch self {
        case .bothEyes: return L10n("Settings::BothEyes")
        case .leftEye: return L10n("Settings::LeftEye")
        case .rightEye: return L10n("Settings::RightEye")
        }
    }
}

enum GearsystemYM2413Option: Int, CaseIterable {
    case automatic
    case disabled

    var title: String {
        switch self {
        case .automatic: return L10n("Settings::Automatic")
        case .disabled: return L10n("Settings::Disabled")
        }
    }
}

enum AppSettings {
    private enum Key {
        static let audioEnabled = "settings.audioEnabled"
        static let hapticsEnabled = "settings.hapticsEnabled"
        static let smoothingEnabled = "settings.smoothingEnabled"
        static let screenSize = "settings.screenSize"
        static let system = "settings.system"
        static let region = "settings.region"
        static let mapper = "settings.mapper"
        static let timing = "settings.timing"
        static let overscan = "settings.overscan"
        static let hideLeftBar = "settings.hideLeftBar"
        static let noSpriteLimitEnabled = "settings.noSpriteLimitEnabled"
        static let glasses = "settings.glasses"
        static let ym2413 = "settings.ym2413"
        static let psgVolume = "settings.psgVolume"
        static let fmVolume = "settings.fmVolume"
        static let saveStateSlot = "settings.saveStateSlot"
    }

    static func registerDefaults() {
        UserDefaults.standard.register(defaults: [
            Key.audioEnabled: true,
            Key.hapticsEnabled: true,
            Key.smoothingEnabled: false,
            Key.screenSize: ScreenSizeOption.fitToWidth.rawValue,
            Key.system: GearsystemSystemOption.automatic.rawValue,
            Key.region: GearsystemRegionOption.automatic.rawValue,
            Key.mapper: GearsystemMapperOption.automatic.rawValue,
            Key.timing: GearsystemTimingOption.automatic.rawValue,
            Key.overscan: GearsystemOverscanOption.disabled.rawValue,
            Key.hideLeftBar: GearsystemHideLeftBarOption.no.rawValue,
            Key.noSpriteLimitEnabled: false,
            Key.glasses: GearsystemGlassesOption.bothEyes.rawValue,
            Key.ym2413: GearsystemYM2413Option.automatic.rawValue,
            Key.psgVolume: 100,
            Key.fmVolume: 100,
            Key.saveStateSlot: 1
        ])
    }

    static var audioEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.audioEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.audioEnabled) }
    }

    static var hapticsEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.hapticsEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.hapticsEnabled) }
    }

    static var smoothingEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.smoothingEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.smoothingEnabled) }
    }

    static var screenSize: ScreenSizeOption {
        get { ScreenSizeOption(rawValue: UserDefaults.standard.integer(forKey: Key.screenSize)) ?? .fitToWidth }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.screenSize) }
    }

    static var system: GearsystemSystemOption {
        get { GearsystemSystemOption(rawValue: UserDefaults.standard.integer(forKey: Key.system)) ?? .automatic }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.system) }
    }

    static var region: GearsystemRegionOption {
        get { GearsystemRegionOption(rawValue: UserDefaults.standard.integer(forKey: Key.region)) ?? .automatic }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.region) }
    }

    static var mapper: GearsystemMapperOption {
        get { GearsystemMapperOption(rawValue: UserDefaults.standard.integer(forKey: Key.mapper)) ?? .automatic }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.mapper) }
    }

    static var timing: GearsystemTimingOption {
        get { GearsystemTimingOption(rawValue: UserDefaults.standard.integer(forKey: Key.timing)) ?? .automatic }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.timing) }
    }

    static var overscan: GearsystemOverscanOption {
        get { GearsystemOverscanOption(rawValue: UserDefaults.standard.integer(forKey: Key.overscan)) ?? .disabled }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.overscan) }
    }

    static var hideLeftBar: GearsystemHideLeftBarOption {
        get { GearsystemHideLeftBarOption(rawValue: UserDefaults.standard.integer(forKey: Key.hideLeftBar)) ?? .no }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.hideLeftBar) }
    }

    static var noSpriteLimitEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.noSpriteLimitEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.noSpriteLimitEnabled) }
    }

    static var glasses: GearsystemGlassesOption {
        get { GearsystemGlassesOption(rawValue: UserDefaults.standard.integer(forKey: Key.glasses)) ?? .bothEyes }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.glasses) }
    }

    static var ym2413: GearsystemYM2413Option {
        get { GearsystemYM2413Option(rawValue: UserDefaults.standard.integer(forKey: Key.ym2413)) ?? .automatic }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.ym2413) }
    }

    static var psgVolume: Int {
        get { min(max(UserDefaults.standard.integer(forKey: Key.psgVolume), 0), 200) }
        set { UserDefaults.standard.set(min(max(newValue, 0), 200), forKey: Key.psgVolume) }
    }

    static var fmVolume: Int {
        get { min(max(UserDefaults.standard.integer(forKey: Key.fmVolume), 0), 200) }
        set { UserDefaults.standard.set(min(max(newValue, 0), 200), forKey: Key.fmVolume) }
    }

    static var saveStateSlot: Int {
        get { min(max(UserDefaults.standard.integer(forKey: Key.saveStateSlot), 1), 5) }
        set { UserDefaults.standard.set(min(max(newValue, 1), 5), forKey: Key.saveStateSlot) }
    }
}
