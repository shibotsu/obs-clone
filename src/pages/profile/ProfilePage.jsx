import { useState, useRef, useEffect } from "react";
import { Image, Spinner, makeStyles } from "@fluentui/react-components";
import { Text, Stack, DefaultButton } from "@fluentui/react";
import { useAuth } from "../../context/AuthContext";
import "./Profile.css";

const useSyles = makeStyles({
  profilePic: {
    objectFit: "cover", // Ensures the image covers the container without stretching
  },
});

const ProfilePage = () => {
  const { user, token, setUser } = useAuth();
  const [profilePic, setProfilePic] = useState(null);
  const [loadingProfile, setLoadingProfile] = useState(false);
  const fileInputRef = useRef(null);

  const profilePicture =
    user.profile_picture === "none" || user.profile_picture.endsWith("/none")
      ? "profile_pic_placeholder.png"
      : user.profile_picture;
  const username = user?.username || "";
  const email = user?.email || "";
  const followers = user?.number_of_followers || 0;

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

      const updatedUser = { ...user, profile_picture: imageUrl };
      setUser(updatedUser);

      setProfilePic(imageUrl);
    } catch (error) {
      alert("Error uploading image: " + error.message);
    } finally {
      setLoadingProfile(false);
    }
  };

  const classes = useSyles();
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
          <Text variant="xxLarge" className="profile-username">
            {username}
          </Text>
          <Text variant="large" className="email-text">
            {email}
          </Text>
          <Text variant="medium" className="follower-count">
            Followers: {followers}
          </Text>
        </Stack>
      )}
    </div>
  );
};

export default ProfilePage;
