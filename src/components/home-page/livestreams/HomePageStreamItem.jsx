import {
  Card,
  Image,
  Caption1,
  Text,
  makeStyles,
} from "@fluentui/react-components";
import { useNavigate } from "react-router-dom";

const useStyles = makeStyles({
  streamCard: {
    width: "100%",
    maxWidth: "320px", // Consistent Twitch-like width
  },
  streamThumbnail: {
    width: "100%",
    height: "180px", // Maintain aspect ratio
    objectFit: "cover", // Crop excess parts
    borderRadius: "8px",
  },
});

const HomePageStreamItem = ({
  id,
  title,
  streamer,
  game,
  viewers,
  thumbnail,
}) => {
  const styles = useStyles();
  let navigate = useNavigate();

  return (
    <Card
      id={id}
      className={styles.streamCard}
      onClick={() => navigate(`/watch/${id}`)}
    >
      <Image src={thumbnail} className={styles.streamThumbnail} />
      <Caption1>{viewers} viewers</Caption1>
      <Text>{title}</Text>
      <Text>{streamer}</Text>
      <Text>{game}</Text>
    </Card>
  );
};

export default HomePageStreamItem;
