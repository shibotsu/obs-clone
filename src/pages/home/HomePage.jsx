import "./Home.css";
import { Text } from "@fluentui/react";
import { Button, makeStyles } from "@fluentui/react-components";
import { HomeSideBar } from "../../components/sidebar/HomeSidebar";
import HomeLiveStreams from "../../components/home-page/livestreams/HomeLiveStreams";
import MostFollowedStreamers from "../../components/home-page/most-followed/MostFollowedStreamers";
import { useAuth } from "../../context/AuthContext";
import { useNavigate } from "react-router-dom";

/* const useStyles = makeStyles({
  livestreamsGrid: {
    display: "grid",
    gap: "16px",
    gridTemplateColumns: "repeat(4, 1fr)", // Default: 4 columns for large screens
    "@media (max-width: 1400px)": {
      gridTemplateColumns: "repeat(3, 1fr)", // 3 columns for medium screens
    },
    "@media (max-width: 1024px)": {
      gridTemplateColumns: "repeat(2, 1fr)", // 2 columns for tablets
    },
    "@media (max-width: 768px)": {
      gridTemplateColumns: "1fr", // 1 column for mobile
    },
  },
}); */

const HomePage = () => {
  const { isLoggedIn, user } = useAuth();
  let navigate = useNavigate();

  return (
    <div className="home-container">
      <div className="livestreams-header">
        <div className="left-group">
          <HomeSideBar className="sidebar-button" />
          <Text variant="xxLarge" className="livestreams-text">
            Livestreams
          </Text>
        </div>
        {isLoggedIn && (
          <Button
            shape="rounded"
            appearance="primary"
            className="start-streaming-button"
            onClick={() => navigate("/start-stream")}
          >
            Start streaming
          </Button>
        )}
      </div>
      <main className="main-page-container">
        <HomeLiveStreams />
        <MostFollowedStreamers />
      </main>
    </div>
  );
};

export default HomePage;
