import {
  Button,
  Menu,
  MenuTrigger,
  MenuPopover,
  MenuList,
  MenuItem,
  Avatar,
  FluentProvider,
  webLightTheme,
  SearchBox,
  makeStyles,
} from "@fluentui/react-components";
import { Link, useNavigate } from "react-router-dom";
import { useAuth } from "../../context/AuthContext";
import { SearchBar } from "./search-bar/SearchBar";
import "./header.css";

const useStyles = makeStyles({
  searchBox: {
    width: "80%",
    maxWidth: "350px",
  },
  logoLink: {
    display: "inline-block",
  },
});

const Header = () => {
  const { isLoggedIn, logout, user } = useAuth();
  let navigate = useNavigate();
  const classes = useStyles();

  console.log(user.profile_picture);

  return (
    <FluentProvider theme={webLightTheme}>
      <header>
        <img
          onClick={() => navigate("/")}
          className="logo"
          src="/logo.png"
          alt="Logo"
        />

        <SearchBar />

        <div className="auth-container">
          {isLoggedIn ? (
            <Menu>
              <MenuTrigger disableButtonEnhancement style={{}}>
                <Avatar
                  name={user.name}
                  image={
                    user.profile_picture &&
                    !user.profile_picture.includes("/none")
                      ? { src: user.profile_picture }
                      : { src: "/profile_pic_placeholder.png" }
                  }
                />
              </MenuTrigger>
              <MenuPopover>
                <MenuList>
                  <MenuItem onClick={() => navigate("/profile")}>
                    Profile
                  </MenuItem>
                  <MenuItem onClick={() => navigate("/settings")}>
                    Settings
                  </MenuItem>
                  <MenuItem onClick={logout}>Log Out</MenuItem>
                </MenuList>
              </MenuPopover>
            </Menu>
          ) : (
            <div>
              <Button
                onClick={() => {
                  navigate("/login");
                }}
                style={{ marginRight: "0.5rem" }}
                appearance="primary"
              >
                Log In
              </Button>
              <Button
                appearance="outline"
                onClick={() => navigate("/register")}
              >
                Sign Up
              </Button>
            </div>
          )}
        </div>
      </header>
    </FluentProvider>
  );
};

export default Header;
