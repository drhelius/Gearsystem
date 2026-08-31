import UIKit
import Combine

final class SettingsViewController: UITableViewController {
    private enum Section: Int, CaseIterable {
        case gameplay
        case system
        case video
        case audio
        case input
        case library
        case about
    }

    private enum GameplayRow: Int, CaseIterable {
        case audio
        case saveStateSlot
    }

    private enum SystemRow: Int, CaseIterable {
        case system
        case region
        case mapper
        case timing
        case ym2413
    }

    private enum VideoRow: Int, CaseIterable {
        case overscan
        case hideLeftBar
        case noSpriteLimit
        case glasses
        case screenSize
        case smoothing
    }

    private enum InputRow: Int, CaseIterable {
        case haptics
    }

    private enum AudioRow: Int, CaseIterable {
        case psgVolume
        case fmVolume
    }

    private enum LibraryRow: Int, CaseIterable {
        case refresh
        case importedRoms
    }

    private var dataStoreSubscriber: AnyCancellable?

    init() {
        super.init(style: .insetGrouped)
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        title = L10n("Common::Settings")
        navigationItem.largeTitleDisplayMode = .always
        navigationController?.navigationBar.prefersLargeTitles = true

        dataStoreSubscriber = dataStore.$allRoms
            .removeDuplicates { $0.count == $1.count }
            .receive(on: RunLoop.main)
            .sink { [weak self] _ in
                self?.tableView.reloadData()
            }
    }

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        tableView.reloadData()
    }

    override func numberOfSections(in tableView: UITableView) -> Int {
        Section.allCases.count
    }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch Section(rawValue: section) {
        case .gameplay: return GameplayRow.allCases.count
        case .system: return SystemRow.allCases.count
        case .video: return VideoRow.allCases.count
        case .audio: return AudioRow.allCases.count
        case .input: return InputRow.allCases.count
        case .library: return LibraryRow.allCases.count
        case .about: return 1
        case nil: return 0
        }
    }

    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch Section(rawValue: section) {
        case .gameplay: return L10n("Settings::Gameplay")
        case .system: return L10n("Settings::System")
        case .video: return L10n("Settings::Video")
        case .audio: return L10n("Settings::AudioSection")
        case .input: return L10n("Settings::Input")
        case .library: return L10n("Settings::Library")
        case .about: return L10n("Settings::About")
        case nil: return nil
        }
    }

    override func tableView(_ tableView: UITableView, titleForFooterInSection section: Int) -> String? {
        switch Section(rawValue: section) {
        case .gameplay: return L10n("Settings::GameplayFooter")
        case .system: return L10n("Settings::SystemFooter")
        case .video: return L10n("Settings::VideoFooter")
        case .audio: return L10n("Settings::AudioMixingFooter")
        case .input: return L10n("Settings::InputFooter")
        default: return nil
        }
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        guard let section = Section(rawValue: indexPath.section) else {
            return UITableViewCell()
        }

        switch section {
        case .gameplay: return gameplayCell(row: indexPath.row)
        case .system: return systemCell(row: indexPath.row)
        case .video: return videoCell(row: indexPath.row)
        case .audio: return audioCell(row: indexPath.row)
        case .input: return inputCell(row: indexPath.row)
        case .library: return libraryCell(row: indexPath.row)
        case .about: return aboutCell()
        }
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)
        guard let section = Section(rawValue: indexPath.section) else { return }

        switch section {
        case .gameplay where GameplayRow(rawValue: indexPath.row) == .saveStateSlot:
            showOptions(
                title: L10n("Settings::SaveStateSlot"),
                options: (1...5).map { String(format: L10n("Settings::SlotFormat"), $0) },
                selectedIndex: AppSettings.saveStateSlot - 1
            ) { AppSettings.saveStateSlot = $0 + 1 }
        case .system:
            showSystemOptions(row: indexPath.row)
        case .video:
            showVideoOptions(row: indexPath.row)
        case .audio:
            showAudioOptions(row: indexPath.row)
        case .library where LibraryRow(rawValue: indexPath.row) == .refresh:
            dataStore.updateAll()
        default:
            break
        }
    }

    private func showSystemOptions(row: Int) {
        switch SystemRow(rawValue: row) {
        case .system:
            showOptions(
                title: L10n("Settings::Console"),
                options: GearsystemSystemOption.allCases.map(\.title),
                selectedIndex: AppSettings.system.rawValue
            ) { AppSettings.system = GearsystemSystemOption(rawValue: $0) ?? .automatic }
        case .region:
            showOptions(
                title: L10n("Settings::Region"),
                options: GearsystemRegionOption.allCases.map(\.title),
                selectedIndex: AppSettings.region.rawValue
            ) { AppSettings.region = GearsystemRegionOption(rawValue: $0) ?? .automatic }
        case .mapper:
            showOptions(
                title: L10n("Settings::Mapper"),
                options: GearsystemMapperOption.allCases.map(\.title),
                selectedIndex: AppSettings.mapper.rawValue
            ) { AppSettings.mapper = GearsystemMapperOption(rawValue: $0) ?? .automatic }
        case .timing:
            showOptions(
                title: L10n("Settings::Timing"),
                options: GearsystemTimingOption.allCases.map(\.title),
                selectedIndex: AppSettings.timing.rawValue
            ) { AppSettings.timing = GearsystemTimingOption(rawValue: $0) ?? .automatic }
        case .ym2413:
            showOptions(
                title: L10n("Settings::YM2413"),
                options: GearsystemYM2413Option.allCases.map(\.title),
                selectedIndex: AppSettings.ym2413.rawValue
            ) { AppSettings.ym2413 = GearsystemYM2413Option(rawValue: $0) ?? .automatic }
        case nil:
            break
        }
    }

    private func showVideoOptions(row: Int) {
        switch VideoRow(rawValue: row) {
        case .overscan:
            showOptions(
                title: L10n("Settings::Overscan"),
                options: GearsystemOverscanOption.allCases.map(\.title),
                selectedIndex: AppSettings.overscan.rawValue
            ) { AppSettings.overscan = GearsystemOverscanOption(rawValue: $0) ?? .disabled }
        case .hideLeftBar:
            showOptions(
                title: L10n("Settings::HideLeftBar"),
                options: GearsystemHideLeftBarOption.allCases.map(\.title),
                selectedIndex: AppSettings.hideLeftBar.rawValue
            ) { AppSettings.hideLeftBar = GearsystemHideLeftBarOption(rawValue: $0) ?? .no }
        case .glasses:
            showOptions(
                title: L10n("Settings::Glasses"),
                options: GearsystemGlassesOption.allCases.map(\.title),
                selectedIndex: AppSettings.glasses.rawValue
            ) { AppSettings.glasses = GearsystemGlassesOption(rawValue: $0) ?? .bothEyes }
        case .screenSize:
            showOptions(
                title: L10n("Settings::ScreenSize"),
                options: ScreenSizeOption.allCases.map(\.title),
                selectedIndex: AppSettings.screenSize.rawValue
            ) { AppSettings.screenSize = ScreenSizeOption(rawValue: $0) ?? .fitToWidth }
        default:
            break
        }
    }

    private func gameplayCell(row: Int) -> UITableViewCell {
        switch GameplayRow(rawValue: row) {
        case .audio:
            return toggleCell(
                title: L10n("Settings::Audio"),
                detail: L10n("Settings::AudioDetail"),
                image: "speaker.wave.2",
                isOn: AppSettings.audioEnabled,
                action: #selector(audioChanged(_:))
            )
        case .saveStateSlot:
            return optionCell(
                title: L10n("Settings::SaveStateSlot"),
                value: String(format: L10n("Settings::SlotFormat"), AppSettings.saveStateSlot),
                image: "square.stack.3d.up"
            )
        case nil:
            return UITableViewCell()
        }
    }

    private func showAudioOptions(row: Int) {
        switch AudioRow(rawValue: row) {
        case .psgVolume:
            showVolumeOptions(title: L10n("Settings::PSGVolume"), currentValue: AppSettings.psgVolume) {
                AppSettings.psgVolume = $0
            }
        case .fmVolume:
            showVolumeOptions(title: L10n("Settings::FMVolume"), currentValue: AppSettings.fmVolume) {
                AppSettings.fmVolume = $0
            }
        case nil:
            break
        }
    }

    private func systemCell(row: Int) -> UITableViewCell {
        switch SystemRow(rawValue: row) {
        case .system:
            return optionCell(title: L10n("Settings::Console"), value: AppSettings.system.summaryTitle, image: "gamecontroller")
        case .region:
            return optionCell(title: L10n("Settings::Region"), value: AppSettings.region.title, image: "globe")
        case .mapper:
            return optionCell(title: L10n("Settings::Mapper"), value: AppSettings.mapper.title, image: "memorychip")
        case .timing:
            return optionCell(title: L10n("Settings::Timing"), value: AppSettings.timing.title, image: "speedometer")
        case .ym2413:
            return optionCell(title: L10n("Settings::YM2413"), value: AppSettings.ym2413.title, image: "pianokeys")
        case nil:
            return UITableViewCell()
        }
    }

    private func videoCell(row: Int) -> UITableViewCell {
        switch VideoRow(rawValue: row) {
        case .overscan:
            return optionCell(title: L10n("Settings::Overscan"), value: AppSettings.overscan.title, image: "rectangle.inset.filled")
        case .hideLeftBar:
            return optionCell(title: L10n("Settings::HideLeftBar"), value: AppSettings.hideLeftBar.title, image: "sidebar.left")
        case .noSpriteLimit:
            return toggleCell(
                title: L10n("Settings::NoSpriteLimit"),
                detail: L10n("Settings::NoSpriteLimitDetail"),
                image: "sparkles.rectangle.stack",
                isOn: AppSettings.noSpriteLimitEnabled,
                action: #selector(noSpriteLimitChanged(_:))
            )
        case .glasses:
            return optionCell(title: L10n("Settings::Glasses"), value: AppSettings.glasses.title, image: "eyeglasses")
        case .screenSize:
            return optionCell(
                title: L10n("Settings::ScreenSize"),
                value: AppSettings.screenSize.title,
                image: "arrow.up.left.and.arrow.down.right"
            )
        case .smoothing:
            return toggleCell(
                title: L10n("Settings::Smoothing"),
                detail: L10n("Settings::SmoothingDetail"),
                image: "square.resize",
                isOn: AppSettings.smoothingEnabled,
                action: #selector(smoothingChanged(_:))
            )
        case nil:
            return UITableViewCell()
        }
    }

    private func inputCell(row: Int) -> UITableViewCell {
        switch InputRow(rawValue: row) {
        case .haptics:
            return toggleCell(
                title: L10n("Settings::Haptics"),
                detail: L10n("Settings::HapticsDetail"),
                image: "hand.tap",
                isOn: AppSettings.hapticsEnabled,
                action: #selector(hapticsChanged(_:))
            )
        case nil:
            return UITableViewCell()
        }
    }

    private func audioCell(row: Int) -> UITableViewCell {
        switch AudioRow(rawValue: row) {
        case .psgVolume:
            return optionCell(
                title: L10n("Settings::PSGVolume"),
                value: "\(AppSettings.psgVolume)%",
                image: "waveform"
            )
        case .fmVolume:
            return optionCell(
                title: L10n("Settings::FMVolume"),
                value: "\(AppSettings.fmVolume)%",
                image: "pianokeys"
            )
        case nil:
            return UITableViewCell()
        }
    }

    private func libraryCell(row: Int) -> UITableViewCell {
        switch LibraryRow(rawValue: row) {
        case .refresh:
            let cell = baseCell(
                title: L10n("Settings::RefreshLibrary"),
                detail: L10n("Settings::RefreshLibraryDetail"),
                image: "arrow.clockwise"
            )
            cell.textLabel?.textColor = view.tintColor
            cell.selectionStyle = .default
            return cell
        case .importedRoms:
            let cell = baseCell(title: L10n("Settings::ImportedRoms"), detail: nil, image: "memorychip")
            cell.detailTextLabel?.text = String(dataStore.allRoms.count)
            return cell
        case nil:
            return UITableViewCell()
        }
    }

    private func aboutCell() -> UITableViewCell {
        let cell = baseCell(title: L10n("Settings::Version"), detail: nil, image: "info.circle")
        let version = Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "-"
        let build = Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as? String ?? "-"
        cell.detailTextLabel?.text = "\(version) (\(build))"
        return cell
    }

    private func baseCell(title: String, detail: String?, image: String) -> UITableViewCell {
        let style: UITableViewCell.CellStyle = detail == nil ? .value1 : .subtitle
        let cell = UITableViewCell(style: style, reuseIdentifier: nil)
        cell.textLabel?.text = title
        cell.textLabel?.adjustsFontSizeToFitWidth = true
        cell.textLabel?.minimumScaleFactor = 0.78
        cell.textLabel?.allowsDefaultTighteningForTruncation = true
        cell.detailTextLabel?.text = detail
        cell.detailTextLabel?.adjustsFontSizeToFitWidth = true
        cell.detailTextLabel?.minimumScaleFactor = 0.75
        cell.imageView?.image = UIImage(systemName: image)
        cell.imageView?.tintColor = view.tintColor
        cell.selectionStyle = .none
        return cell
    }

    private func optionCell(title: String, value: String, image: String, isEnabled: Bool = true) -> UITableViewCell {
        let cell = UITableViewCell(style: .value1, reuseIdentifier: nil)
        cell.textLabel?.text = title
        cell.textLabel?.adjustsFontSizeToFitWidth = true
        cell.textLabel?.minimumScaleFactor = 0.78
        cell.textLabel?.allowsDefaultTighteningForTruncation = true
        cell.detailTextLabel?.text = value
        cell.detailTextLabel?.adjustsFontSizeToFitWidth = true
        cell.detailTextLabel?.minimumScaleFactor = 0.7
        cell.imageView?.image = UIImage(systemName: image)
        cell.accessoryType = .disclosureIndicator
        cell.selectionStyle = isEnabled ? .default : .none
        applyEnabledState(isEnabled, to: cell)
        return cell
    }

    private func toggleCell(
        title: String,
        detail: String,
        image: String,
        isOn: Bool,
        action: Selector,
        isEnabled: Bool = true
    ) -> UITableViewCell {
        let cell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        cell.textLabel?.text = title
        cell.textLabel?.adjustsFontSizeToFitWidth = true
        cell.textLabel?.minimumScaleFactor = 0.78
        cell.textLabel?.allowsDefaultTighteningForTruncation = true
        cell.detailTextLabel?.text = detail
        cell.detailTextLabel?.adjustsFontSizeToFitWidth = true
        cell.detailTextLabel?.minimumScaleFactor = 0.75
        cell.imageView?.image = UIImage(systemName: image)
        cell.selectionStyle = .none

        let toggle = UISwitch()
        toggle.isOn = isOn
        toggle.isEnabled = isEnabled
        toggle.accessibilityLabel = title
        toggle.addTarget(self, action: action, for: .valueChanged)
        cell.accessoryView = toggle
        applyEnabledState(isEnabled, to: cell)
        return cell
    }

    private func applyEnabledState(_ isEnabled: Bool, to cell: UITableViewCell) {
        cell.isUserInteractionEnabled = isEnabled
        cell.textLabel?.textColor = isEnabled ? .label : .tertiaryLabel
        cell.detailTextLabel?.textColor = isEnabled ? .secondaryLabel : .tertiaryLabel
        cell.imageView?.tintColor = isEnabled ? view.tintColor : .tertiaryLabel
    }

    private func showOptions(
        title: String,
        options: [String],
        selectedIndex: Int,
        onSelection: @escaping (Int) -> Void
    ) {
        let controller = OptionSelectionViewController(
            title: title,
            optionTitles: options,
            selectedIndex: selectedIndex,
            onSelection: onSelection
        )
        navigationController?.pushViewController(controller, animated: true)
    }

    private func showVolumeOptions(title: String, currentValue: Int, onSelection: @escaping (Int) -> Void) {
        let values = Array(stride(from: 0, through: 200, by: 10))
        let selectedIndex = values.enumerated().min(by: {
            abs($0.element - currentValue) < abs($1.element - currentValue)
        })?.offset ?? 10
        showOptions(
            title: title,
            options: values.map { "\($0)%" },
            selectedIndex: selectedIndex
        ) { onSelection(values[$0]) }
    }

    @objc private func audioChanged(_ sender: UISwitch) {
        AppSettings.audioEnabled = sender.isOn
    }

    @objc private func noSpriteLimitChanged(_ sender: UISwitch) {
        AppSettings.noSpriteLimitEnabled = sender.isOn
    }

    @objc private func smoothingChanged(_ sender: UISwitch) {
        AppSettings.smoothingEnabled = sender.isOn
    }

    @objc private func hapticsChanged(_ sender: UISwitch) {
        AppSettings.hapticsEnabled = sender.isOn
    }

}
