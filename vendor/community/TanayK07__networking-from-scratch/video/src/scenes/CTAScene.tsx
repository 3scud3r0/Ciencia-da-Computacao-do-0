import { AbsoluteFill, useCurrentFrame, spring, useVideoConfig, interpolate } from "remotion";

export const CTAScene: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();

  const logoScale = spring({
    frame,
    fps,
    config: { damping: 12, stiffness: 60 },
  });

  const urlOpacity = interpolate(frame, [20, 35], [0, 1], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

  const badgeOpacity = interpolate(frame, [40, 55], [0, 1], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

  const starPulse = spring({
    frame: frame - 60,
    fps,
    config: { damping: 8, stiffness: 40 },
  });

  return (
    <AbsoluteFill
      style={{
        backgroundColor: "#0d1117",
        justifyContent: "center",
        alignItems: "center",
      }}
    >
      {/* GitHub icon (simplified) */}
      <div style={{ transform: `scale(${logoScale})`, marginBottom: 30 }}>
        <svg
          width="80"
          height="80"
          viewBox="0 0 24 24"
          fill="#e6edf3"
        >
          <path d="M12 0C5.37 0 0 5.37 0 12c0 5.31 3.435 9.795 8.205 11.385.6.105.825-.255.825-.57 0-.285-.015-1.23-.015-2.235-3.015.555-3.795-.735-4.035-1.41-.135-.345-.72-1.41-1.23-1.695-.42-.225-1.02-.78-.015-.795.945-.015 1.62.87 1.845 1.23 1.08 1.815 2.805 1.305 3.495.99.105-.78.42-1.305.765-1.605-2.67-.3-5.46-1.335-5.46-5.925 0-1.305.465-2.385 1.23-3.225-.12-.3-.54-1.53.12-3.18 0 0 1.005-.315 3.3 1.23.96-.27 1.98-.405 3-.405s2.04.135 3 .405c2.295-1.56 3.3-1.23 3.3-1.23.66 1.65.24 2.88.12 3.18.765.84 1.23 1.905 1.23 3.225 0 4.605-2.805 5.625-5.475 5.925.435.375.81 1.095.81 2.22 0 1.605-.015 2.895-.015 3.3 0 .315.225.69.825.57A12.02 12.02 0 0024 12c0-6.63-5.37-12-12-12z" />
        </svg>
      </div>

      {/* URLs */}
      <div
        style={{
          opacity: urlOpacity,
          textAlign: "center",
          marginBottom: 30,
        }}
      >
        <div
          style={{
            fontSize: 30,
            fontWeight: 700,
            fontFamily: "'JetBrains Mono', monospace",
            color: "#00897B",
            marginBottom: 12,
          }}
        >
          networkingfromscratch.vercel.app
        </div>
        <div
          style={{
            fontSize: 22,
            fontWeight: 500,
            fontFamily: "'JetBrains Mono', monospace",
            color: "#8b949e",
          }}
        >
          github.com/TanayK07/networking-from-scratch
        </div>
      </div>

      {/* Badges */}
      <div
        style={{
          display: "flex",
          gap: 24,
          opacity: badgeOpacity,
          marginBottom: 50,
        }}
      >
        {["Free", "MIT License", "No signup"].map((badge) => (
          <div
            key={badge}
            style={{
              padding: "10px 24px",
              borderRadius: 30,
              border: "1px solid #30363d",
              backgroundColor: "#161b22",
              fontSize: 20,
              fontFamily: "'Inter', sans-serif",
              color: "#8b949e",
            }}
          >
            {badge}
          </div>
        ))}
      </div>

      {/* Star CTA */}
      <div
        style={{
          transform: `scale(${0.8 + starPulse * 0.2})`,
          display: "flex",
          alignItems: "center",
          gap: 12,
          padding: "16px 40px",
          borderRadius: 12,
          backgroundColor: "#00897B",
          boxShadow: `0 0 ${30 * starPulse}px rgba(0,137,123,0.4)`,
        }}
      >
        <span style={{ fontSize: 28 }}>⭐</span>
        <span
          style={{
            fontSize: 26,
            fontWeight: 700,
            fontFamily: "'Inter', sans-serif",
            color: "#ffffff",
          }}
        >
          Star on GitHub
        </span>
      </div>
    </AbsoluteFill>
  );
};
