import { useState, useRef } from "react";
import { Image, Button } from "@fluentui/react-components";
import { Text, Stack, DefaultButton } from "@fluentui/react";
import { useAuth } from "../../context/AuthContext";
import "./Profile.css";

const ProfilePage = () => {
  const { token } = useAuth();
  const [profilePic, setProfilePic] = useState(null);
  const [followers, setFollowers] = useState(120);

  const username = "JohnDoe";
  const email = "johndoe@gmail.com";
  const fileInputRef = useRef(null);

  const handleProfilePicChange = (event) => {
    const file = event.target.files[0];
    if (file) {
      const imageUrl = URL.createObjectURL(file);
      setProfilePic(imageUrl);
    }
  };

  return (
    <div className="profile-container">
      <Stack tokens={{ childrenGap: 20 }} className="profile-card">
        <div className="profile-pic-container">
          <Image
            src={profilePic || "profile_pic_placeholder.png"}
            shape="rounded"
            alt="Profile picture"
            width={150}
            height={150}
            className="profile-pic"
          />
          <input type="file" accept="image/*" className="file-input" />
          <DefaultButton
            text="Change Picture"
            onClick={() => fileInputRef.current.onClick()}
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
    </div>
  );
};

export default ProfilePage;
