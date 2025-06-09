import { FollowButton } from "../../components/follow-button/FollowButton";
import { useQuery } from "@tanstack/react-query";
import { useParams } from "react-router-dom";
import {
  Image,
  Spinner,
  Tab,
  TabList,
  makeStyles,
} from "@fluentui/react-components";
import { Text, Stack } from "@fluentui/react";
import { useAuth } from "../../context/AuthContext";
import "./Profile.css";
import { useState } from "react";

const useStyles = makeStyles({
  profilePic: {
    objectFit: "cover", // Ensures the image covers the container without stretching
  },
  // Tab styles
  tabsContainer: {
    marginTop: "20px",
    width: "100%",
    maxWidth: "800px",
    marginLeft: "auto",
    marginRight: "auto",
    display: "flex",
    justifyContent: "center",
    flexDirection: "column",
  },

  tabContent: {
    padding: "20px",
    minHeight: "200px",
    width: "100%",
    backgroundColor: "#fafafa",
    borderRadius: "8px",
    marginTop: "10px",
  },

  aboutMeContainer: {
    display: "flex",
    flexDirection: "column",
    gap: "15px",
  },

  aboutMeHeader: {
    display: "flex",
    justifyContent: "space-between",
    alignItems: "center",
  },

  aboutMeText: {
    lineHeight: "1.6",
    color: "#666",
    textAlign: "left",
  },

  emptyState: {
    color: "#999",
    fontStyle: "italic",
    textAlign: "center",
    padding: "40px 20px",
  },

  editButtons: {
    display: "flex",
    gap: "10px",
  },

  textareaStyle: {
    marginBottom: "10px",
    width: "100%",
  },
});

const StreamerProfilePage = () => {
  const { id } = useParams();
  const { user, token } = useAuth();
  const classes = useStyles();
  const [aboutMeText, setAboutMeText] = useState("");
  const [selectedTab, setSelectedTab] = useState("livestreams");

  const { data, isLoading, error } = useQuery({
    queryFn: () =>
      fetch(`http://157.230.16.67:8000/api/profile/${id}`, {
        headers: {
          "Content-Type": "application/json",
        },
      }).then((res) => res.json()),
    queryKey: ["streamerProfile", id],
  });

  if (isLoading) {
    return <Spinner style={{ marginTop: "10rem" }} />;
  }

  if (error || !data || !data.channel) {
    return <div>An error occured.</div>;
  }

  const { username, profile_picture, number_of_followers } = data.channel;
  const profilePic =
    profile_picture?.endsWith("/none") || profile_picture === "none"
      ? "/profile_pic_placeholder.png"
      : profile_picture;

  return (
    <div className="profile-container">
      {console.log(JSON.stringify(data))}
      {console.log("Profile pic: " + profilePic)}
      <Stack tokens={{ childrenGap: 20 }} className="profile-card">
        <div className="profile-pic-container">
          <Image
            src={profilePic}
            shape="circular"
            alt="Profile picture"
            width={150}
            height={150}
            className="profile-pic"
          />
        </div>
        <Text variant="xxLarge" className="profile-username">
          {username}
        </Text>
        <Text variant="medium" className="follower- count">
          Followers: {number_of_followers ?? 0}
        </Text>
        {user?.id && user.id === Number(id) ? null : <FollowButton id={id} />}
        <div className={classes.tabsContainer}>
          <TabList
            selectedValue={selectedTab}
            onTabSelect={(event, data) => setSelectedTab(data.value)}
          >
            <Tab value="livestreams">Previous Livestreams</Tab>
            <Tab value="about">About Me</Tab>
          </TabList>
        </div>
      </Stack>
      <div className={classes.tabContent}>
        {selectedTab === "livestreams" && (
          <div className={classes.emptyState}>
            <Text size={400}>No previous livestreams yet.</Text>
          </div>
        )}

        {selectedTab === "about" && (
          <div className={classes.aboutMeContainer}>
            <div className={classes.aboutMeHeader}>
              <Text size={500} weight="semibold">
                About Me
              </Text>
            </div>
            <div>
              {aboutMeText ? (
                <Text className={classes.aboutMeText}>{aboutMeText}</Text>
              ) : (
                <div className={classes.emptyState}>
                  <Text size={400}>No information added yet.</Text>
                </div>
              )}
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default StreamerProfilePage;
