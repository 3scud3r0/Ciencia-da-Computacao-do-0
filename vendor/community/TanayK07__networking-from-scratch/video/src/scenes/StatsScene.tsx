import { AbsoluteFill, useCurrentFrame, interpolate, spring, useVideoConfig } from "remotion";

const AnimatedNumber: React.FC<{ target: number; label: string; delay: number }> = ({
  target,
  label,
  delay,
}) => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();

  const progress = spring({
    frame: frame - delay,
    fps,
    config: { damping: 30, stiffness: 80 },
  });

  const value = Math.round(target * progress);

  const opacity = interpolate(frame - delay, [0, 10], [0, 1], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

  return (
    <div style={{ textAlign: "center", opacity }}>
      <div
        style={{
          fontSize: 96,
          fontWeight: 800,
          fontFamily: "'JetBrains Mono', monospace",
          color: "#00897B",
          lineHeight: 1,
        }}
      >
        {value}
      </div>
      <div
        style={{
          fontSize: 28,
          fontWeight: 500,
          fontFamily: "'Inter', sans-serif",
          color: "#8b949e",
          marginTop: 8,
        }}
      >
        {label}
      </div>
    </div>
  );
};

export const StatsScene: React.FC = () => {
  const frame = useCurrentFrame();

  return (
    <AbsoluteFill
      style={{
        backgroundColor: "#0d1117",
        justifyContent: "center",
        alignItems: "center",
      }}
    >
      <div
        style={{
          display: "flex",
          gap: 100,
          alignItems: "center",
        }}
      >
        <AnimatedNumber target={289} label="lessons" delay={0} />
        <div
          style={{
            width: 2,
            height: 80,
            backgroundColor: "#21262d",
            opacity: interpolate(frame, [10, 20], [0, 1], {
              extrapolateLeft: "clamp",
              extrapolateRight: "clamp",
            }),
          }}
        />
        <AnimatedNumber target={15} label="phases" delay={15} />
        <div
          style={{
            width: 2,
            height: 80,
            backgroundColor: "#21262d",
            opacity: interpolate(frame, [25, 35], [0, 1], {
              extrapolateLeft: "clamp",
              extrapolateRight: "clamp",
            }),
          }}
        />
        <div
          style={{
            textAlign: "center",
            opacity: interpolate(frame - 30, [0, 15], [0, 1], {
              extrapolateLeft: "clamp",
              extrapolateRight: "clamp",
            }),
          }}
        >
          <div
            style={{
              fontSize: 56,
              fontWeight: 800,
              fontFamily: "'JetBrains Mono', monospace",
              color: "#00897B",
            }}
          >
            C
          </div>
          <div
            style={{
              fontSize: 28,
              fontWeight: 500,
              fontFamily: "'Inter', sans-serif",
              color: "#8b949e",
              marginTop: 8,
            }}
          >
            & Python
          </div>
        </div>
      </div>
    </AbsoluteFill>
  );
};
