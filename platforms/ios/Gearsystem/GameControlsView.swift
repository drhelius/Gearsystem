import UIKit

final class GameControlsView: UIView {
    var onButtonChanged: ((GearsystemButton, Bool) -> Void)? {
        didSet {
            actionOne.onButtonChanged = onButtonChanged
            actionTwo.onButtonChanged = onButtonChanged
            start.onButtonChanged = onButtonChanged
        }
    }

    var hapticsEnabled = true {
        didSet {
            dPad.hapticsEnabled = hapticsEnabled
            actionOne.hapticsEnabled = hapticsEnabled
            actionTwo.hapticsEnabled = hapticsEnabled
            start.hapticsEnabled = hapticsEnabled
        }
    }

    let dPad = DirectionPadView()
    let actionOne = GameControlButton(title: "1", button: .one, shape: .circle)
    let actionTwo = GameControlButton(title: "2", button: .two, shape: .circle)
    let start = GameControlButton(title: "START", button: .start, shape: .capsule)

    private let actionGuide = UILayoutGuide()
    private var portraitConstraints = [NSLayoutConstraint]()
    private var landscapeConstraints = [NSLayoutConstraint]()
    private var portraitBottomConstraints = [NSLayoutConstraint]()
    private var usingLandscapeConstraints = false

    override init(frame: CGRect) {
        super.init(frame: frame)
        configure()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        configure()
    }

    override func layoutSubviews() {
        let landscape = bounds.width > bounds.height
        if landscape != usingLandscapeConstraints {
            usingLandscapeConstraints = landscape
            NSLayoutConstraint.deactivate(landscape ? portraitConstraints : landscapeConstraints)
            NSLayoutConstraint.activate(landscape ? landscapeConstraints : portraitConstraints)
        }

        super.layoutSubviews()
    }

    func positionPortraitControls(after screenBottom: CGFloat) -> Bool {
        guard UIDevice.current.userInterfaceIdiom == .pad,
              bounds.height > bounds.width,
              dPad.bounds.height > 0.0,
              let primaryConstraint = portraitBottomConstraints.first else { return false }

        let minimumGap: CGFloat = 44.0
        let minimumBottomInset: CGFloat = 24.0
        let safeFrame = safeAreaLayoutGuide.layoutFrame
        let bottomInset = max(
            safeFrame.maxY - screenBottom - minimumGap - dPad.bounds.height,
            minimumBottomInset
        )
        let constant = -bottomInset
        guard abs(primaryConstraint.constant - constant) > 0.5 else { return false }

        for constraint in portraitBottomConstraints {
            constraint.constant = constant
        }
        return true
    }

    private func configure() {
        isMultipleTouchEnabled = true
        backgroundColor = .clear
        dPad.onDirectionChanged = { [weak self] direction, pressed in
            guard let self else { return }
            self.onButtonChanged?(self.emulatorButton(for: direction), pressed)
        }

        dPad.translatesAutoresizingMaskIntoConstraints = false
        actionOne.translatesAutoresizingMaskIntoConstraints = false
        actionTwo.translatesAutoresizingMaskIntoConstraints = false
        start.translatesAutoresizingMaskIntoConstraints = false

        addSubview(dPad)
        addSubview(actionOne)
        addSubview(actionTwo)
        addSubview(start)
        addLayoutGuide(actionGuide)

        let isPad = UIDevice.current.userInterfaceIdiom == .pad
        let dPadSize: CGFloat = isPad ? 176.0 : 132.0
        let actionSize: CGFloat = isPad ? 88.0 : 72.0
        let actionOffset: CGFloat = isPad ? 36.0 : 30.0
        let primaryOffset: CGFloat = isPad ? 220.0 : 120.0
        let menuWidth: CGFloat = isPad ? 104.0 : 80.0
        let menuHeight: CGFloat = 44.0
        let portraitBottomInset: CGFloat = isPad ? 160.0 : 24.0
        let portraitPrimaryConstraint = isPad
            ? dPad.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor, constant: -portraitBottomInset)
            : dPad.centerYAnchor.constraint(equalTo: safeAreaLayoutGuide.centerYAnchor, constant: primaryOffset)
        let portraitStartConstraint = start.bottomAnchor.constraint(
            equalTo: safeAreaLayoutGuide.bottomAnchor,
            constant: -portraitBottomInset
        )

        if isPad {
            portraitBottomConstraints = [portraitPrimaryConstraint, portraitStartConstraint]
        }

        NSLayoutConstraint.activate([
            dPad.widthAnchor.constraint(equalToConstant: dPadSize),
            dPad.heightAnchor.constraint(equalTo: dPad.widthAnchor),

            actionOne.widthAnchor.constraint(equalToConstant: actionSize),
            actionOne.heightAnchor.constraint(equalTo: actionOne.widthAnchor),

            actionTwo.trailingAnchor.constraint(equalTo: actionOne.leadingAnchor, constant: -14.0),
            actionTwo.centerYAnchor.constraint(equalTo: actionOne.centerYAnchor, constant: actionOffset),
            actionTwo.widthAnchor.constraint(equalTo: actionOne.widthAnchor),
            actionTwo.heightAnchor.constraint(equalTo: actionTwo.widthAnchor),

            start.widthAnchor.constraint(equalToConstant: menuWidth),
            start.heightAnchor.constraint(equalToConstant: menuHeight),

            actionGuide.leadingAnchor.constraint(equalTo: actionTwo.leadingAnchor),
            actionGuide.trailingAnchor.constraint(equalTo: actionOne.trailingAnchor)
        ])

        portraitConstraints = [
            dPad.leadingAnchor.constraint(equalTo: safeAreaLayoutGuide.leadingAnchor, constant: 20.0),
            actionOne.trailingAnchor.constraint(equalTo: safeAreaLayoutGuide.trailingAnchor, constant: -20.0),
            portraitPrimaryConstraint,
            actionOne.centerYAnchor.constraint(equalTo: dPad.centerYAnchor, constant: -(actionOffset * 0.5)),
            start.centerXAnchor.constraint(equalTo: safeAreaLayoutGuide.centerXAnchor),
            portraitStartConstraint
        ]

        landscapeConstraints = [
            dPad.leadingAnchor.constraint(equalTo: safeAreaLayoutGuide.leadingAnchor, constant: 8.0),
            actionOne.trailingAnchor.constraint(equalTo: safeAreaLayoutGuide.trailingAnchor, constant: -8.0),
            dPad.centerYAnchor.constraint(equalTo: safeAreaLayoutGuide.centerYAnchor),
            actionOne.centerYAnchor.constraint(equalTo: safeAreaLayoutGuide.centerYAnchor, constant: -(actionOffset * 0.5)),
            start.centerXAnchor.constraint(equalTo: actionGuide.centerXAnchor),
            start.bottomAnchor.constraint(
                equalTo: safeAreaLayoutGuide.bottomAnchor,
                constant: isPad ? -24.0 : -8.0
            )
        ]

        NSLayoutConstraint.activate(portraitConstraints)
    }

    private func emulatorButton(for direction: DirectionPadDirection) -> GearsystemButton {
        switch direction {
        case .up: return .up
        case .down: return .down
        case .left: return .left
        case .right: return .right
        }
    }
}

final class GameControlButton: UIButton {
    enum Shape {
        case circle
        case capsule
    }

    var onButtonChanged: ((GearsystemButton, Bool) -> Void)?
    var hapticsEnabled = true

    private let emulatorButton: GearsystemButton
    private let shape: Shape
    private var pressed = false
    private let feedback = UIImpactFeedbackGenerator(style: .light)

    init(title: String, button: GearsystemButton, shape: Shape) {
        self.emulatorButton = button
        self.shape = shape
        super.init(frame: .zero)

        setTitle(title, for: .normal)
        setTitleColor(.label, for: .normal)
        titleLabel?.font = shape == .circle
            ? .systemFont(ofSize: 24.0, weight: .bold)
            : .systemFont(ofSize: 11.0, weight: .semibold)
        backgroundColor = UIColor.secondarySystemFill.withAlphaComponent(0.92)
        layer.borderColor = UIColor.separator.withAlphaComponent(0.65).cgColor
        layer.borderWidth = 1.0
        accessibilityLabel = title

        addTarget(self, action: #selector(press), for: [.touchDown, .touchDragEnter])
        addTarget(self, action: #selector(releaseButton), for: [.touchUpInside, .touchUpOutside, .touchCancel, .touchDragExit])
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func layoutSubviews() {
        super.layoutSubviews()
        layer.cornerRadius = shape == .circle ? bounds.width * 0.5 : bounds.height * 0.5
    }

    @objc private func press() {
        guard !pressed else { return }
        pressed = true
        if hapticsEnabled {
            feedback.prepare()
            feedback.impactOccurred(intensity: 0.55)
        }
        backgroundColor = tintColor.withAlphaComponent(0.28)
        transform = CGAffineTransform(scaleX: 0.94, y: 0.94)
        onButtonChanged?(emulatorButton, true)
    }

    @objc private func releaseButton() {
        guard pressed else { return }
        pressed = false
        backgroundColor = UIColor.secondarySystemFill.withAlphaComponent(0.92)
        transform = .identity
        onButtonChanged?(emulatorButton, false)
    }
}
