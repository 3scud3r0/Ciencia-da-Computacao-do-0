import "./index.css";
import { Composition } from "remotion";
import { PromoVideo } from "./PromoVideo";

const FPS = 30;
const DURATION = 35 * FPS;

export const RemotionRoot: React.FC = () => {
  return (
    <>
      <Composition
        id="Square"
        component={PromoVideo}
        durationInFrames={DURATION}
        fps={FPS}
        width={1080}
        height={1080}
      />
      <Composition
        id="Landscape"
        component={PromoVideo}
        durationInFrames={DURATION}
        fps={FPS}
        width={1920}
        height={1080}
      />
      <Composition
        id="Vertical"
        component={PromoVideo}
        durationInFrames={DURATION}
        fps={FPS}
        width={1080}
        height={1920}
      />
    </>
  );
};
