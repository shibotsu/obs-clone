import {
  Button,
  Input,
  Menu,
  MenuTrigger,
  MenuPopover,
  MenuList,
  MenuItem,
  Avatar,
  FluentProvider,
  webLightTheme,
} from "@fluentui/react-components";
import { Link, useNavigate } from "react-router-dom";
import { useAuth } from "../../authentication/AuthContext";
import "./header.css";

const Header = () => {
  const { isLoggedIn, logout } = useAuth();
  let navigate = useNavigate();

  return (
    <FluentProvider theme={webLightTheme}>
      <header>
        <Link to="/">
          <img className="logo" src="logo.png" alt="Logo" />
        </Link>

        <Input placeholder="Search streams..." className="search-bar" />

        <div className="auth-container">
          {isLoggedIn ? (
            <Menu>
              <MenuTrigger disableButtonEnhancement style={{}}>
                <Avatar name="User" size={32} className="avatar" />
              </MenuTrigger>
              <MenuPopover>
                <MenuList>
                  <MenuItem>Profile</MenuItem>
                  <MenuItem>Settings</MenuItem>
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
