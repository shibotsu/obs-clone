import { Text } from "@fluentui/react";
import { makeStyles, Image } from "@fluentui/react-components";
import { Link } from "react-router-dom";

const useStyles = makeStyles({
  item: {
    display: "flex",
    flexDirection: "row",
    alignItems: "center",
    width: "100%",
    marginBottom: "1px",
    borderRadius: "8px",
  },
  image: {
    maxWidth: "35px",
    minWidth: "35px",
    minHeight: "35px",
    margin: "8px",
    position: "relative",
    left: "0px",
    marginRight: "12px",
  },
  link: {
    textDecoration: "none",
    color: "inherit",
    display: "block",
    borderRadius: "8px",
    // Ensure that any inner elements don't inherit unwanted text decoration
    "& *": {
      textDecoration: "none",
    },
    "&:hover": {
      backgroundColor: "#f0f0f0", // Adjust this value for the desired darkening effect
      textDecoration: "none",
    },
  },
});

const FollowingSidebarItem = ({ id, avatar, name }) => {
  const classes = useStyles();

  const image =
    avatar === "none" || avatar.endsWith("none")
      ? "profile_pic_placeholder.png"
      : avatar;

  return (
    <Link className={classes.link} to={`/streamer-profile/${id}`}>
      <div className={classes.item}>
        <Image
          src={image}
          fit="cover"
          shape="circular"
          className={classes.image}
        />
        <Text variant="medium">{name}</Text>
      </div>
    </Link>
  );
};

export default FollowingSidebarItem;
