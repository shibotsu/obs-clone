import "./Home.css";
import { Text } from "@fluentui/react";
import { Button, makeStyles } from "@fluentui/react-components";
import HomePageStreamItem from "../../components/home-page/HomePageStreamItem";

const livestreams = [
  {
    id: 1,
    title: "LOS RATONES COMPETITIVE SPRING SPLIT",
    streamer: "Caedrel",
    game: "League of Legends",
    viewers: "28.7K viewers",
    thumbnail:
      "https://i.pcmag.com/imagery/reviews/05i45Hf8jvSS87RYAWXWgeW-4.fit_scale.size_1028x578.v1569474054.jpg",
  },
  {
    id: 2,
    title: "NEW! DROPS 15:00 UTC",
    streamer: "skill4ltu",
    game: "World of Tanks",
    viewers: "2.6K viewers",
    thumbnail:
      "https://mnd-assets.mynewsdesk.com/image/upload/ar_16:9,c_fill,dpr_auto,f_auto,g_xy_center,q_auto:good,w_1782,x_960,y_540/df79edcspiyqkdkd9zq09d",
  },
  {
    id: 3,
    title: "D0cC CFG TODAY",
    streamer: "d0cc_tv",
    game: "Counter-Strike",
    viewers: "935 viewers",
    thumbnail:
      "https://csmarket.gg/blog/wp-content/uploads/2024/02/0cc382629edec933916a6f9912bd0f24-_1_-1536x864.webp",
  },
  {
    id: 4,
    title: "NEW* TOKEN DROPS",
    streamer: "QuickyBaby",
    game: "World of Tanks",
    viewers: "3.7K viewers",
    thumbnail:
      "https://preview.redd.it/dfi4kogtoug71.png?width=1080&crop=smart&auto=webp&s=e4a22e31c6751c6af4000510d17c47f214244f4d",
  },
  {
    id: 5,
    title: "LOS RATONES COMPETITIVE SPRING SPLIT",
    streamer: "Caedrel",
    game: "League of Legends",
    viewers: "28.7K viewers",
    thumbnail:
      "https://i.pcmag.com/imagery/reviews/05i45Hf8jvSS87RYAWXWgeW-4.fit_scale.size_1028x578.v1569474054.jpg",
  },
  {
    id: 6,
    title: "NEW! DROPS 15:00 UTC",
    streamer: "skill4ltu",
    game: "World of Tanks",
    viewers: "2.6K viewers",
    thumbnail:
      "https://mnd-assets.mynewsdesk.com/image/upload/ar_16:9,c_fill,dpr_auto,f_auto,g_xy_center,q_auto:good,w_1782,x_960,y_540/df79edcspiyqkdkd9zq09d",
  },
  {
    id: 7,
    title: "D0cC CFG TODAY",
    streamer: "d0cc_tv",
    game: "Counter-Strike",
    viewers: "935 viewers",
    thumbnail:
      "https://csmarket.gg/blog/wp-content/uploads/2024/02/0cc382629edec933916a6f9912bd0f24-_1_-1536x864.webp",
  },
  {
    id: 8,
    title: "NEW* TOKEN DROPS",
    streamer: "QuickyBaby",
    game: "World of Tanks",
    viewers: "3.7K viewers",
    thumbnail:
      "https://preview.redd.it/dfi4kogtoug71.png?width=1080&crop=smart&auto=webp&s=e4a22e31c6751c6af4000510d17c47f214244f4d",
  },
];

const useStyles = makeStyles({
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
});

const HomePage = () => {
  const classes = useStyles();
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
      <main className="main-page-container">
        <div className={classes.livestreamsGrid}>
          {livestreams.map((livestream) => (
            <HomePageStreamItem
              id={livestream.id}
              title={livestream.title}
              streamer={livestream.streamer}
              game={livestream.game}
              viewers={livestream.viewers}
              thumbnail={livestream.thumbnail}
            />
          ))}
        </div>
      </main>
    </div>
  );
};

export default HomePage;
