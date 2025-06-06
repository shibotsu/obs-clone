import { useState, useRef, useEffect } from "react";
import {
  Image,
  Spinner,
  makeStyles,
  TabList,
  Tab,
  Text,
  Button,
  Textarea,
} from "@fluentui/react-components";
import { Text as FluentText, Stack, DefaultButton } from "@fluentui/react";
import {
  Edit24Regular,
  Save24Regular,
  Dismiss24Regular,
} from "@fluentui/react-icons";
import { useAuth } from "../../context/AuthContext";
import FollowersDialog from "../../components/channel-followers/FollowersDialog";
import "./Profile.css";

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

const ProfilePage = () => {
  const classes = useStyles();
  const { user, token, setUser } = useAuth();
  const [profilePic, setProfilePic] = useState(null);
  const [loadingProfile, setLoadingProfile] = useState(false);
  const [selectedTab, setSelectedTab] = useState("livestreams");
  const [aboutMeText, setAboutMeText] = useState(user?.about_me || "");
  const [isEditingAbout, setIsEditingAbout] = useState(false);
  const [tempAboutText, setTempAboutText] = useState("");
  const fileInputRef = useRef(null);

  const profilePicture =
    user.profile_picture === "none" || user.profile_picture.endsWith("/none")
      ? "profile_pic_placeholder.png"
      : user.profile_picture;
  const username = user?.username || "";
  const email = user?.email || "";
  const followers = user?.number_of_followers || 0;
  const id = user?.id || "";

  const handleProfilePicChange = async (event) => {
    const file = event.target.files[0];
    if (!file) return;

    if (!file.type.startsWith("image/")) {
      alert("Please select a valid image file.");
      return;
    }

    const maxSize = 5 * 1024 * 1024;
    if (file.size > maxSize) {
      alert("File size exceedes 5MB limit.");
      return;
    }

    setLoadingProfile(true);

    try {
      const formData = new FormData();
      formData.append("profile_picture", file);

      const response = await fetch("http://127.0.0.1:8000/api/picture", {
        method: "POST",
        headers: {
          Authorization: `Bearer ${token}`,
        },
        body: formData,
      });

      if (!response.ok) {
        throw new Error("Upload failed. Please try again.");
      }

      const data = await response.json();

      const imageUrl = data.profile_picture || URL.createObjectURL(file);

      console.log("ImageURL: " + imageUrl);

      const updatedUser = { ...user, profile_picture: imageUrl };
      setUser(updatedUser);

      setProfilePic(imageUrl);
    } catch (error) {
      alert("Error uploading image: " + error.message);
    } finally {
      setLoadingProfile(false);
    }
  };

  const handleEditAbout = () => {
    setTempAboutText(aboutMeText);
    setIsEditingAbout(true);
  };

  const handleSaveAbout = async () => {
    try {
      // You can implement API call here to save the about me text
      // const response = await fetch(`http://127.0.0.1:8000/api/user/about`, {
      //   method: 'PUT',
      //   headers: {
      //     'Authorization': `Bearer ${token}`,
      //     'Content-Type': 'application/json',
      //   },
      //   body: JSON.stringify({ about_me: tempAboutText }),
      // });

      setAboutMeText(tempAboutText);
      setIsEditingAbout(false);

      // Update user context if needed
      const updatedUser = { ...user, about_me: tempAboutText };
      setUser(updatedUser);
    } catch (error) {
      alert("Error saving about me: " + error.message);
    }
  };

  const handleCancelEdit = () => {
    setTempAboutText("");
    setIsEditingAbout(false);
  };

  return (
    <div className="profile-container">
      {console.log(JSON.stringify(user))}
      {console.log(JSON.parse(localStorage.getItem("user")))}
      {loadingProfile ? (
        <div className="spinner-container">
          <Spinner />
        </div>
      ) : (
        <Stack tokens={{ childrenGap: 20 }} className="profile-card">
          <div className="profile-pic-container">
            <Image
              src={profilePicture}
              shape="circular"
              alt="Profile picture"
              width={150}
              height={150}
              className={classes.profilePic}
            />
            <input
              type="file"
              accept="image/*"
              className="file-input"
              ref={fileInputRef}
              onChange={handleProfilePicChange}
            />
            <DefaultButton
              text="Change Picture"
              onClick={() => fileInputRef.current.click()}
              className="change-pic-btn"
            />
          </div>
          <div style={{ textAlign: "center", width: "100%" }}>
            <Text
              size={900} // Use size instead of variant for more reliable sizing
              weight="semibold"
              className="profile-username"
              style={{
                display: "block",
                textAlign: "center",
                fontSize: "2rem",
                marginBottom: "8px",
              }}
            >
              {username}
            </Text>
            <Text
              size={600}
              className="email-text"
              style={{
                display: "block",
                textAlign: "center",
                fontSize: "1.25rem",
                color: "#666",
                marginBottom: "15px",
              }}
            >
              {email}
            </Text>
          </div>
          <FollowersDialog followers={followers} userId={id} />
        </Stack>
      )}
      <div className={classes.tabsContainer}>
        <TabList
          selectedValue={selectedTab}
          onTabSelect={(event, data) => setSelectedTab(data.value)}
        >
          <Tab value="livestreams">Previous Livestreams</Tab>
          <Tab value="about">About Me</Tab>
        </TabList>

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
                {!isEditingAbout && (
                  <Button
                    appearance="subtle"
                    icon={<Edit24Regular />}
                    onClick={handleEditAbout}
                  >
                    Edit
                  </Button>
                )}
              </div>

              {isEditingAbout ? (
                <div>
                  <Textarea
                    value={tempAboutText}
                    onChange={(e) => setTempAboutText(e.target.value)}
                    placeholder="Tell us about yourself..."
                    rows={6}
                    resize="vertical"
                    className={classes.textareaStyle}
                  />
                  <div className={classes.editButtons}>
                    <Button
                      appearance="primary"
                      icon={<Save24Regular />}
                      onClick={handleSaveAbout}
                    >
                      Save
                    </Button>
                    <Button
                      appearance="secondary"
                      icon={<Dismiss24Regular />}
                      onClick={handleCancelEdit}
                    >
                      Cancel
                    </Button>
                  </div>
                </div>
              ) : (
                <div>
                  {aboutMeText ? (
                    <Text className={classes.aboutMeText}>{aboutMeText}</Text>
                  ) : (
                    <div className={classes.emptyState}>
                      <Text size={400}>
                        No information added yet. Click Edit to add your story!
                      </Text>
                    </div>
                  )}
                </div>
              )}
            </div>
          )}
        </div>
      </div>
    </div>
  );
};

export default ProfilePage;
