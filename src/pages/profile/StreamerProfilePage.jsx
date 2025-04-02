import { useState } from "react";
import { Image } from "@fluentui/react-components";
import { Text, Stack } from "@fluentui/react";
import { useAuth } from "../../context/AuthContext";
import "./Profile.css";

const StreamerProfilePage = () => {
  const { token } = useAuth();
  const [profilePic, setProfilePic] = useState(null);
  const [followers, setFollowers] = useState(120);

  const username = "JohnDoe";

  return (
    <div className="profile-container">
      <Stack tokens={{ childrenGap: 20 }} className="profile-card">
        <div className="profile-pic-container">
          <Image
            src={profilePic || "profile_pic_placeholder.png"}
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
        <Text variant="medium" className="follower-count">
          Followers: {followers}
        </Text>
      </Stack>
    </div>
  );
};

export default StreamerProfilePage;
