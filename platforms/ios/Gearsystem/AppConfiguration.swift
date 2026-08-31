import Foundation

enum AppConfiguration {
    static let libraryTitleLocalizationKey = "Common::Gearsystem"
    static let thumbnailBaseURL = URL(string: "https://www.drhelius.com/thumbnails/gearsystem/")!

    static func romCRC(inArchiveAt url: URL) -> String? {
        GearsystemEmulator.romCRC(inArchiveAt: url)
    }
}
