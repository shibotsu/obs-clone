import "./Home.css";
import { Text } from "@fluentui/react";
import { Button } from "@fluentui/react-components";

const HomePage = () => {
  return (
    <div className="home-container">
      <div className="livestreams-header">
        <Text variant="xxLarge" className="livestreams-text">
          Livestreams
        </Text>
        <Button
          shape="rounded"
          appearance="primary"
          className="start-streaming-button"
        >
          Start streaming
        </Button>
      </div>
    </div>
  );
};

export default HomePage;
