import { Image, Text, makeStyles } from "@fluentui/react-components";

const useClasses = makeStyles({
  streamerContainer: {
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    justifyContent: "center",
    margin: "2px",
    marginTop: "10px",
    padding: "10px",
    borderRadius: "20px",
    transition: "background-color 0.2s ease, transform 0.1s ease",
    cursor: "pointer",

    ":hover": {
      backgroundColor: "#f3f3f3",
    },
    ":active": {
      transform: "scale(0.97)",
      backgroundColor: "#e0e0e0",
    },
  },
  profilePic: {
    objectFit: "cover",
  },
});

const HomePageStreamerItem = ({ username, profilePic, onClick }) => {
  const classes = useClasses();
  return (
    <div className={classes.streamerContainer} onClick={onClick}>
      <Image
        src={profilePic}
        shape="circular"
        shadow
        height={110}
        width={110}
        className={classes.profilePic}
      />
      <Text size={400}>{username}</Text>
    </div>
  );
};

export default HomePageStreamerItem;
