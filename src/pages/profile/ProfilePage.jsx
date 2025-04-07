import { useState, useRef } from "react";
import { Image } from "@fluentui/react-components";
import { Text, Stack, DefaultButton } from "@fluentui/react";
import { useAuth } from "../../context/AuthContext";
import "./Profile.css";

const ProfilePage = () => {
  const { token } = useAuth();
  const [profilePic, setProfilePic] = useState(null);
  const [followers, setFollowers] = useState(120);
  const [uploading, setUploading] = useState(false);

  const username = "JohnDoe";
  const email = "johndoe@gmail.com";
  const fileInputRef = useRef(null);

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

    const imageUrl = URL.createObjectURL(file);

    setProfilePic(imageUrl);

    /*setUploading(true);

    try {
      const formData = new FormData();
      formData.append("profilePic", file);

      const response = await fetch("https://example.com/api/uploadProfiePic", {
        method: "POST",
        body: formData,
      });

      if (!response.ok) {
        throw new Error("Upload failed. Please try again.");
      }

      const data = await response.json();

      const imageUrl = data.imageUrl || URL.createObjectURL(file);
      setProfilePic(imageUrl);
    } catch (error) {
      alert("Error uploading image: " + error.message);
    } finally {
      setUploading(false);
    }*/
  };

  return (
    <div className="profile-container">
      <Stack tokens={{ childrenGap: 20 }} className="profile-card">
        <div className="profile-pic-container">
          {console.log(token)}
          <Image
            src={profilePic || "profile_pic_placeholder.png"}
            shape="circular"
            alt="Profile picture"
            width={150}
            height={150}
            className="profile-pic"
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
    </div>
  );
};

export default ProfilePage;
